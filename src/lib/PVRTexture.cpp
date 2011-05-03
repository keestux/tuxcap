#include <assert.h>
#include "PVRTexture.h"
#include "Common.h"
#include "SexyAppBase.h"
#include "PakInterface.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

enum
{
    kPVRTextureFlagType565 = 2,
    kPVRTextureFlagTypeOGL1555 = 17,        // OGL_RGBA_5551
    kPVRTextureFlagTypeOGL565 = 19,         // OGL_RGB_565
    kPVRTextureFlagTypePVRTC_2 = 24,
    kPVRTextureFlagTypePVRTC_4 = 25,
};

#if TARGET_OS_IPHONE == 0
// FIXME. Get this from an include file. Perhaps PVRTexLib.h
// Values for glCompressedTexImage2D, from OpenGLES/ESn/glext.h
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif

#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

using namespace Sexy;
using namespace std;

typedef struct _PVRTexHeader
{
    uint32_t headerLength;
    uint32_t height;
    uint32_t width;
    uint32_t numMipmaps;
    uint32_t flags;
    uint32_t dataLength;
    uint32_t bpp;
    uint32_t bitmaskRed;
    uint32_t bitmaskGreen;
    uint32_t bitmaskBlue;
    uint32_t bitmaskAlpha;
    uint32_t pvrTag;
    uint32_t numSurfs;
} PVRTexHeader;

static const char gPVRTexIdentifier[] = "PVR!";

PVRTexture::PVRTexture() :
        mPVRTextureFlagType(0)
{
    mImageData.clear();
    mImageDataLength.clear();
}

PVRTexture::~PVRTexture()
{
    for (size_t i = 0; i < mImageData.size(); i++) {
        delete [] mImageData[i];
    }
}

bool PVRTexture::unpackPVRData(uint8_t* data)
{
    // Forget about endianness, just accept LE.
    const PVRTexHeader * header = (const PVRTexHeader *)data;

    // Sanity check
    if (header->headerLength != sizeof(PVRTexHeader)) {
        // TODO. Throw exception
        return false;
    }
    if (header->pvrTag != *(uint32_t*)gPVRTexIdentifier) {
        // TODO. Throw exception
        return false;
    }

    mPVRTextureFlagType = header->flags & PVR_TEXTURE_FLAG_TYPE_MASK;
    mHasAlpha = header->bitmaskAlpha != 0 ? true : false;

    // We only accept a limited set of formats
    switch (mPVRTextureFlagType) {
#if TARGET_OS_IPHONE
    case kPVRTextureFlagTypePVRTC_4:
    case kPVRTextureFlagTypePVRTC_2:
#endif
    case kPVRTextureFlagType565:
    case kPVRTextureFlagTypeOGL565:
    case kPVRTextureFlagTypeOGL1555:
        break;
    default:
        // Just return false, to signal caller "we failed to read"
        return false;
    }

    mWidth = header->width;
    mHeight = header->height;

    // The following loop is to load MIPmaps, but we are (probably) just interested in the first.
    int width = mWidth;
    int height = mHeight;
    uint32_t dataOffset = sizeof(PVRTexHeader);
    int blockSize = 0;
    int widthBlocks = 0;
    int heightBlocks = 0;
    int dataSize;
    while (dataOffset < header->dataLength) {
        // Do something
        if (mPVRTextureFlagType == kPVRTextureFlagTypePVRTC_4) {
            blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
            widthBlocks = width / 4;
            heightBlocks = height / 4;
            // Clamp to minimum number of blocks
            if (widthBlocks < 2)
                widthBlocks = 2;
            if (heightBlocks < 2)
                heightBlocks = 2;
            dataSize = widthBlocks * heightBlocks * ((blockSize * 4) / 8);
        }
        else if (mPVRTextureFlagType == kPVRTextureFlagTypePVRTC_2) {
            blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
            widthBlocks = width / 8;
            heightBlocks = height / 4;
            // Clamp to minimum number of blocks
            if (widthBlocks < 2)
                widthBlocks = 2;
            if (heightBlocks < 2)
                heightBlocks = 2;
            dataSize = widthBlocks * heightBlocks * ((blockSize * 2) / 8);
        }
        else if (mPVRTextureFlagType == kPVRTextureFlagType565
                || mPVRTextureFlagType == kPVRTextureFlagTypeOGL565
                || mPVRTextureFlagType == kPVRTextureFlagTypeOGL1555) {
            dataSize = width * height * 2;
        }

        uint8_t * blob = new uint8_t[dataSize];
        memcpy(blob, data + dataOffset, dataSize);
        mImageData.push_back(blob);
        mImageDataLength.push_back(dataSize);

        dataOffset += dataSize;

        width = std::max(width >> 1, 1);
        height = std::max(height >> 1, 1);
    }

    return true;
}

bool PVRTexture::initWithContentsOfFile(const string& fname)
{
    string myFname = gSexyAppBase->GetAppResourceFileName(fname);
    PFILE* file = p_fopen(myFname.c_str(), "r");
    if (file == NULL) {
        // TODO. Throw exception
        return false;
    }

    int size = p_size(file);
    uint8_t* buffer = new uint8_t[size];
    int res = p_fread((void*)buffer, sizeof(uint8_t), size * sizeof(*buffer), file);

    if (size != res) {
        delete[] buffer;
        // TODO. Throw exception
        return false;
    }

    if (!unpackPVRData(buffer)) {
        delete [] buffer;
        // Just return false, to signal caller "we failed to read"
        return false;
    }

    delete [] buffer;
    return true;
}

// Copy 2 bytes per pixel into a newly allocated buffer
static uint8_t * get_rect_2(uint8_t * data, int width, int height, int x, int y, int w, int h)
{
    int	bpp = 2;		// bytes per pixel
    uint8_t * dst = new uint8_t[w * h * bpp];
    for (int j = 0; j < h && j < height; j++) {
	uint8_t * srcrow = &data[((j + y) * width + x) * bpp];
        int nrbytes = w * bpp;
        if (x + w > width) {
            nrbytes = (width - x) * bpp;
        }
	memcpy(&dst[(j * w * bpp)], srcrow, nrbytes);
    }
    return dst;
}

GLuint PVRTexture::CreateTexture(int x, int y, int w, int h)
{
    // We only account for the first data image in the PVR (no MIPmaps)
    uint8_t * data = mImageData[0];
    int datalength = mImageDataLength[0];

    /* Create an OpenGL texture for thing */
    GLuint texture;
    GLint glerr;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // These are the same as in MemoryImage::CreateTexture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1);
    GLenum internalFormat;
    switch (mPVRTextureFlagType) {
    case kPVRTextureFlagTypePVRTC_4:
        if (!(x == 0 && y == 0)) {
            // Not sure what to do here.
            assert(0);
        }
        if (mHasAlpha)
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        else
            internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, datalength, data);
        break;
    case kPVRTextureFlagTypePVRTC_2:
        if (!(x == 0 && y == 0)) {
            // Not sure what to do here.
            assert(0);
        }
        if (mHasAlpha)
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        else
            internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, datalength, data);
        break;
    case kPVRTextureFlagType565:
    case kPVRTextureFlagTypeOGL565:
    {
        // Copy the data for this rect
        uint8_t * data2 = get_rect_2(data, mWidth, mHeight, x, y, w, h);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data2);
        delete [] data2;
    }
        break;
    case kPVRTextureFlagTypeOGL1555:
    {
        // Copy the data for this rect
        uint8_t * data2 = get_rect_2(data, mWidth, mHeight, x, y, w, h);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, data2);
        delete [] data2;
    }
        break;
    default:
        assert(0);
        break;
    }
    glerr = glGetError();
    // TODO. Check error code.

    return texture;
}
