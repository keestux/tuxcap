#define XMD_H

#include <SDL_image.h>
#include "ImageLib.h"
#include <math.h>
#include "Common.h"
#include "SexyAppBase.h"
#include "PakInterface.h"
#include "Logging.h"

using namespace ImageLib;

ImageLib::Image::Image()
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("image");             // Same as Image.h
    TLOG(mLogFacil, 1, "new ImageLib::Image");
#endif
    mWidth = 0;
    mHeight = 0;
    mBits = NULL;
}

ImageLib::Image::Image(int width, int height)
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("image");             // Same as Image.h
    TLOG(mLogFacil, 1, Logger::format("new ImageLib::Image(%d, %d)", width, height));
#endif
    mWidth = width;
    mHeight = height;
    mBits = new Uint32[mWidth * mHeight];
}

ImageLib::Image::~Image()
{
    delete[] mBits;
}

int ImageLib::Image::GetWidth()
{
    return mWidth;
}

int ImageLib::Image::GetHeight()
{
    return mHeight;
}

Uint32* ImageLib::Image::GetBits()
{
    return mBits;
}

int ImageLib::gAlphaComposeColor = 0x00FFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

static inline Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = ((Uint8 *)surface->pixels) + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        // Is there a color lookup table?
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

static ImageLib::Image* loadImageFromSDLSurface(SDL_Surface* surface)
{
    ImageLib::Image* anImage = new ImageLib::Image(surface->w, surface->h);

    TLOG(anImage->mLogFacil, 1, Logger::format("loadImageFromSDLSurface, surface=%p, palette=%p", surface, surface->format ? surface->format->palette : 0));

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    SDL_PixelFormat* fmt = surface->format;
    if (fmt->BytesPerPixel == 4 && fmt->Rshift == 16 && fmt->Rloss == 0 && fmt->Gshift == 8 && fmt->Gloss == 0 && fmt->Bshift == 0 && fmt->Bloss == 0) {
        // We can copy the the 32 bits pixel data straightforward
        int y_stride = surface->pitch/4;
        if (y_stride == surface->w) {
            // There are no gaps between the rows
            memcpy(anImage->mBits, surface->pixels, surface->w * surface->h * 4);
        }
        else {
            for (int y = 0; y < surface->h; ++y) {
                for (int x = 0; x < surface->w; ++x) {
                    anImage->mBits[y * anImage->mWidth + x] = ((Uint32*)surface->pixels)[y * y_stride + x];
                }
            }
        }
    }
    else {
        for (int y = 0; y < surface->h; ++y) {
            for (int x = 0; x < surface->w; ++x) {
                Uint32 pixel;
                Uint8 red, green, blue, alpha;
                pixel = getpixel(surface, x, y);
                SDL_GetRGBA(pixel, fmt, &red, &green, &blue, &alpha);
                anImage->mBits[y * anImage->mWidth + x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
            }
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    return anImage;
}

static SDL_Surface * read_pakfile(const std::string & theFilename)
{
    // Read a file into image object

    PFILE* file = p_fopen(theFilename.c_str(), "r");
    if (file == NULL)
        return NULL;

    int size = GetPakPtr()->FSize(file);
    Uint8* buffer = new Uint8[size];
    int res = p_fread((void*)buffer, sizeof(Uint8), size * sizeof(Uint8), file);

    if (size != res) {
        delete[] buffer;
        return NULL;
    }

    SDL_RWops* rw = SDL_RWFromMem(buffer, size);
    SDL_Surface* surface = IMG_Load_RW(rw, 0);
    SDL_FreeRW(rw);
    delete[] buffer;
    p_fclose(file);

    return surface;
}

ImageLib::Image* ImageLib::GetImage(std::string theFilename, bool lookForAlphaImage)
{
    if (!gAutoLoadAlpha)
        lookForAlphaImage = false;

    theFilename = Sexy::ReplaceBackSlashes(theFilename);

    size_t aLastDotPos = theFilename.rfind('.');
    size_t aLastSlashPos = theFilename.rfind('/');

    std::string anExt;
    std::string aFilename;

    if (aLastDotPos != std::string::npos && (aLastSlashPos == std::string::npos || aLastDotPos > aLastSlashPos)) {
        anExt = theFilename.substr(aLastDotPos);
        aFilename = theFilename.substr(0, aLastDotPos);
    } else {
        aFilename = theFilename;
    }

    SDL_Surface* surface = NULL;
    bool ok = false;

    bool pak = GetPakPtr()->isLoaded();

    if (anExt.length() == 0) {
        // No extension given, try a couple
        std::list<std::string> coderList;
        coderList.push_back("png");
        coderList.push_back("jpg");
        coderList.push_back("gif");
        coderList.push_back("PNG");
        coderList.push_back("JPG");
        coderList.push_back("GIF");
        for (std::list<std::string>::const_iterator entry = coderList.begin(); entry != coderList.end(); entry++) {
            if (pak) {
                // Read a file into image object
                surface = read_pakfile(theFilename + "." + *entry);
            } else {
                std::string myFilename = theFilename + "." + *entry;
                if (Sexy::gSexyAppBase->FileExists(myFilename)) {
                    surface = IMG_Load(myFilename.c_str());
                }
            }

            if (surface) {
                ok = true;
                break;
            }
        }
    } else {

        // Filename with extension
        if (pak) {
            surface = read_pakfile(theFilename);
        } else {
            if (Sexy::gSexyAppBase->FileExists(theFilename)) {
                surface = IMG_Load(theFilename.c_str());
            }
        }

        if (!surface)
            return NULL;
        ok = true;
    }

    Image* anAlphaImage = NULL;
    if (lookForAlphaImage) {
        // Check for alpha images. Use the filename WITHOUT the extension!
        // Check FileName_
        anAlphaImage = GetImage(aFilename + "_", false);

        if (anAlphaImage == NULL) {
            // Check <dirname> / _<basename>
            anAlphaImage = GetImage(aFilename.substr(0, aLastSlashPos + 1) + "_" + aFilename.substr(aLastSlashPos + 1), false);
        }
    }

    ImageLib::Image* anImage = NULL;
    // Compose alpha channel with image
    if (anAlphaImage != NULL) {

        if (surface != NULL) {
            anImage = loadImageFromSDLSurface(surface);
        }

        if (anImage != NULL) {
            // Merge the alpha with the regular image
            if ((anImage->mWidth == anAlphaImage->mWidth) &&
                    (anImage->mHeight == anAlphaImage->mHeight)) {
                Uint32* aBits1 = anImage->mBits;
                Uint32* aBits2 = anAlphaImage->mBits;
                int aSize = anImage->mWidth * anImage->mHeight;

                for (int i = 0; i < aSize; i++) {
                    *aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
                    ++aBits1;
                    ++aBits2;
                }
            }

            delete anAlphaImage;
        } else if (gAlphaComposeColor == 0x00FFFFFF) {

            // Hoping that the compiler would optimize this, by using a const 0x00FFFFFF
            anImage = anAlphaImage;
            Uint32* aBits1 = anImage->mBits;

            int aSize = anImage->mWidth * anImage->mHeight;
            for (int i = 0; i < aSize; i++) {
                *aBits1 = 0x00FFFFFF | ((*aBits1 & 0xFF) << 24);
                ++aBits1;
            }
        } else {

            int aColor = gAlphaComposeColor;
            anImage = anAlphaImage;
            Uint32* aBits1 = anImage->mBits;

            int aSize = anImage->mWidth * anImage->mHeight;
            for (int i = 0; i < aSize; i++) {
                *aBits1 = aColor | ((*aBits1 & 0xFF) << 24);
                ++aBits1;
            }
        }
    }

    if (anImage == NULL && ok) {
        anImage = loadImageFromSDLSurface(surface);
    }

    if (surface != NULL)
        SDL_FreeSurface(surface);

    return anImage;
}
