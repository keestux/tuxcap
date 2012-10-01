#include <assert.h>
#include "DXTTexture.h"
#include "Common.h"
#include "SexyAppBase.h"
#include "PakInterface.h"
#include "GLState.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif
#include "Logging.h"

#include "GLExtensions.h"

using namespace Sexy;
using namespace std;


DXTTexture::DXTTexture():mDXTTextureFlagType(0)
{
    mImageData.clear();
    mImageDataLength.clear();

    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("dxttexture");
    TLOG(mLogFacil, 1, "new DXTTexture");
#endif
}

DXTTexture::~DXTTexture()
{
    for (size_t i = 0; i < mImageData.size(); i++) {
        delete [] mImageData[i];
    }
}

bool DXTTexture::unpackDXTData(uint8_t* data)
{
    TLOG(mLogFacil, 1, "unpackDXTData");

    /*  variables       */
    DDS_header header;
    unsigned int buffer_index = 0;
    /*  file reading variables  */
    unsigned int DDS_main_size;
    unsigned int width, height;
    int uncompressed, block_size = 16;
    unsigned int flag;
    if (NULL == data)
    {
        /*      we can't do it! */
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 1");
        return false;
    }
    /*  try reading in the header       */
    memcpy ( (void*)(&header), (const void *)data, sizeof( DDS_header ) );
    buffer_index = sizeof( DDS_header );
    /*  validate the header     */
    flag = ('D'<<0)|('D'<<8)|('S'<<16)|(' '<<24);
    if (header.dwMagic != flag) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 2");
        return false;
    }
    if (header.dwSize != 124) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 3");
        return false;
    }
    /*  I need all of these     */
    flag = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if ((header.dwFlags & flag) != flag) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 4");
        return false;
    }
    /*  According to the MSDN spec, the dwFlags should contain
        DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
        uncompressed.  Some DDS writers do not conform to the
        spec, so I need to make my reader more tolerant */
    /*  I need one of these     */
    flag = DDPF_FOURCC | DDPF_RGB;
    if ((header.sPixelFormat.dwFlags & flag) == 0) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 5");
        return false;
    }
    if (header.sPixelFormat.dwSize != 32) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 6");
        return false;
    }

    if ((header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0) {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 7");
        return false;
    }

    /*  make sure it is a type we can upload    */
    if ((header.sPixelFormat.dwFlags & DDPF_FOURCC) &&
        !(
            (header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))) ||
            (header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('3'<<24))) ||
            (header.sPixelFormat.dwFourCC == (('D'<<0)|('X'<<8)|('T'<<16)|('5'<<24)))
          ))
    {
        TLOG(mLogFacil, 1, "FAILED unpackDXTData 8");
        return false;
    }

    width = header.dwWidth;
    height = header.dwHeight;
    mPow2 = true;
    mSquare = true;
    mWidth = width;
    mHeight =height;
    TLOG(mLogFacil, 1, Logger::format("Width=%d, height=%d", width, height));
    uncompressed = 1 - (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
    if (uncompressed)
    {
        TLOG(mLogFacil, 1, "Uncompressed");
        mDXTTextureFlagType = GL_RGB;
        block_size = 3;
        if (header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
        {
            mDXTTextureFlagType= GL_RGBA;
            block_size = 4;
        }
        DDS_main_size = width * height * block_size;
    } else
    {
        /*      well, we know it is DXT1/3/5, because we checked above  */
        switch ((header.sPixelFormat.dwFourCC >> 24) - '0')
        {
        case 1:
            TLOG(mLogFacil, 1, "Compressed DXT1");
            mDXTTextureFlagType = COMPRESSED_RGB_S3TC_DXT1_EXT;
            block_size = 8;
            break;
        case 3:
            TLOG(mLogFacil, 1, "Compressed DXT3");
            mDXTTextureFlagType = COMPRESSED_RGBA_S3TC_DXT3_EXT;
            block_size = 16;
            break;
        case 5:
            TLOG(mLogFacil, 1, "Compressed DXT5");
            mDXTTextureFlagType = COMPRESSED_RGBA_S3TC_DXT5_EXT;
            block_size = 16;
            break;
        }
        DDS_main_size = ((width+3)>>2)*((height+3)>>2)*block_size;
    }
    TLOG(mLogFacil, 1, Logger::format("Size=%d", DDS_main_size));

    uint8_t * blob = new uint8_t[DDS_main_size];
    memcpy(blob, (const void*)(&data[buffer_index]), DDS_main_size);
    mImageData.push_back(blob);
    mImageDataLength.push_back(DDS_main_size);

    TLOG(mLogFacil, 1, "Unpack Succesfull");
    return true;
}

bool DXTTexture::initWithContentsOfFile(const string& fname)
{
    TLOG(mLogFacil, 1, "initWithContentsOfFile");

    string myFname = gSexyAppBase->GetAppResourceFileName(fname);
    PFILE* file = p_fopen(myFname.c_str(), "r");
    if (file == NULL) {
        // TODO. Throw exception
        TLOG(mLogFacil, 1, "FAILED initWithContentsOfFile 1");
        return false;
    }

    int size = p_size(file);
    uint8_t* buffer = new uint8_t[size];
    int res = p_fread((void*)buffer, sizeof(uint8_t), size * sizeof(*buffer), file);

    p_fclose(file);

    if (size != res) {
        delete[] buffer;
        // TODO. Throw exception
        TLOG(mLogFacil, 1, "FAILED initWithContentsOfFile 2");
        return false;
    }

    if (!unpackDXTData(buffer)) {
        delete [] buffer;
        // Just return false, to signal caller "we failed to read"
        TLOG(mLogFacil, 1, "FAILED initWithContentsOfFile 3");
        return false;
    }
    delete[] buffer;
    return true;
}

// Copy 2 bytes per pixel into a newly allocated buffer
static uint8_t * get_rect_2(uint8_t * data, int width, int height, int x, int y, int w, int h)
{
    int bpp = 2;                // bytes per pixel
    int dstlen = w * h * bpp;
    uint8_t * dst = new uint8_t[dstlen];
    memset(dst, 0, dstlen);
    for (int j = 0; j < h && (y + j) < height; j++) {
        uint8_t * srcrow = &data[((j + y) * width + x) * bpp];
        int nrbytes = w * bpp;
        if (x + w > width) {
            nrbytes = (width - x) * bpp;
        }
        memcpy(&dst[(j * w * bpp)], srcrow, nrbytes);
    }
    return dst;
}

GLuint DXTTexture::CreateTexture(int x, int y, int w, int h)
{
    TLOG(mLogFacil, 1, "CreateTexture");

    // We only account for the first data image in the DXT (no MIPmaps)
    if (mImageData.size() == 0) {
        assert(0);
    }
    uint8_t * data = mImageData[0];
    int datalength = mImageDataLength[0];

    /* Create an OpenGL texture for the thing */
    GLuint texture;
    GLint glerr;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // These are the same as in MemoryImage::CreateTexture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCompressedTexImage2D(GL_TEXTURE_2D,0,
                           mDXTTextureFlagType, 256, 256, 0,
                           datalength, data );

    glerr = glGetError();
    TLOG(mLogFacil, 1, Logger::format("glerr=%d", glerr));

    return texture;
}

void DXTTexture::DoPurgeBits()
{
    // This is called after CreateTexture

    // Delete the data buffers
    for (size_t i = 0; i < mImageData.size(); i++) {
        delete [] mImageData[i];
    }

    mImageData.clear();
    mImageDataLength.clear();
}
