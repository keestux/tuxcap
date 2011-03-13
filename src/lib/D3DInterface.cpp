#include "D3DInterface.h"
#include "TextureData.h"
#include "DDInterface.h"
#include "Graphics.h"
#include "Color.h"
#if 0
#include "DirectXErrorString.h"
#endif
#include "SexyMatrix.h"
#include "SexyAppBase.h"
#include "TriVertex.h"
#include <assert.h>
#include <algorithm>
#include <vector>

using namespace Sexy;
using namespace std;

static bool gLinearFilter = false;

std::string D3DInterface::mErrorString;
#if 0
static const int gVertexType = D3DFVF_TLVERTEX;
#endif

static void SetLinearFilter(bool linearFilter)
{
    if (gLinearFilter != linearFilter) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearFilter ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linearFilter ? GL_LINEAR : GL_NEAREST);
        gLinearFilter = linearFilter;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if 0
static SDL_Surface* CreateTextureSurface(int theWidth, int theHeight/*, PixelFormat theFormat*/)
{

    SDL_Surface* aSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, theWidth, theHeight, 32, SDL_rmask, SDL_gmask, SDL_bmask, SDL_amask);

    return aSurface;
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::FillOldCursorAreaTexture(GLint x, GLint y)
{
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, 64, 64);
}

void D3DInterface::BltOldCursorArea(GLfloat x, GLfloat y, const Color& theColor)
{
    glDisable(GL_BLEND);

    //SetLinearFilter(false);

    SexyRGBA rgba = theColor.ToRGBA();
                        
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);

    D3DTLVERTEX aVertex[4] = 
    {
         {0.0f, 1.0f, rgba, x,          y,          0},
         {0.0f, 0.0f, rgba, x,          y + 64,     0},
         {1.0f, 0.0f, rgba, x + 64,     y + 64,     0},
         {1.0f, 1.0f, rgba, x + 64,     y,          0},
    };

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].tu));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_BLEND);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if 0
static void CopySurface8888ToImage(void *theDest, Uint32 theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight)
{
    char *srcRow = (char*) theDest;
    uint32_t *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

    for (int y = 0; y < theHeight; y++) {
        uint32_t *src = (uint32_t*) srcRow;
        uint32_t *dst = dstRow;

        for (int x = 0; x < theWidth; x++)
            *dst++ = *src++;

        dstRow += theImage->GetWidth();
        srcRow += theDestPitch;
    }
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VertexList {

    enum {
        MAX_STACK_VERTS = 100
    };
    D3DTLVERTEX mStackVerts[MAX_STACK_VERTS];
    D3DTLVERTEX *mVerts;
    int mSize;
    int mCapacity;

    typedef int size_type;

    VertexList() : mVerts(mStackVerts), mSize(0), mCapacity(MAX_STACK_VERTS)
    {
    }

    VertexList(const VertexList & theList) : mVerts(mStackVerts), mSize(theList.mSize), mCapacity(MAX_STACK_VERTS)
    {
        reserve(mSize);
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    ~VertexList()
    {
        if (mVerts != mStackVerts)
            delete mVerts;
    }

    void reserve(int theCapacity)
    {
        if (mCapacity < theCapacity) {
            mCapacity = theCapacity;
            D3DTLVERTEX *aNewList = new D3DTLVERTEX[theCapacity];
            memcpy(aNewList, mVerts, mSize * sizeof (mVerts[0]));
            if (mVerts != mStackVerts)
                delete mVerts;

            mVerts = aNewList;
        }
    }

    void push_back(const D3DTLVERTEX & theVert)
    {
        if (mSize == mCapacity)
            reserve(mCapacity * 2);

        mVerts[mSize++] = theVert;
    }

    void operator=(const VertexList & theList)
    {
        reserve(theList.mSize);
        mSize = theList.mSize;
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    D3DTLVERTEX & operator[](int thePos)
    {
        return mVerts[thePos];
    }

    int size()
    {
        return mSize;
    }

    void clear()
    {
        mSize = 0;
    }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline float GetCoord(const D3DTLVERTEX &theVertex, int theCoord)
{
    switch (theCoord) {
    case 0: return theVertex.sx;
    case 1: return theVertex.sy;
    case 2: return theVertex.sz;
    case 3: return theVertex.tu;
    case 4: return theVertex.tv;
    default: return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static inline D3DTLVERTEX Interpolate(const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, float t)
{
    D3DTLVERTEX aVertex = v1;
    aVertex.sx = v1.sx + t * (v2.sx - v1.sx);
    aVertex.sy = v1.sy + t * (v2.sy - v1.sy);
    aVertex.tu = v1.tu + t * (v2.tu - v1.tu);
    aVertex.tv = v1.tv + t * (v2.tv - v1.tv);
    Color c1(v1.color);
    Color c2(v2.color);
    if (c1 != c2) {
        // TODO ???? Don't we have to clip to 255?
        int r = c1.mRed   + (int) (t * (c2.mRed   - c1.mRed));
        int g = c1.mGreen + (int) (t * (c2.mGreen - c1.mGreen));
        int b = c1.mBlue  + (int) (t * (c2.mBlue  - c1.mBlue));
        int a = c1.mAlpha + (int) (t * (c2.mAlpha - c1.mAlpha));

        aVertex.color = Color(r, g, b, a).ToRGBA();
    }

    return aVertex;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

D3DInterface::D3DInterface()
{
    mLogFacil = LoggerFacil::find("d3dinterface");
    Logger::tlog(mLogFacil, 1, "new D3DInterface");

#if 0
    mHWnd = NULL;
    mDD = NULL;
    mD3D = NULL;
    mD3DDevice = NULL;
#endif
    mWidth = 640;
    mHeight = 480;
    //mIsWindowed = true;                 // FIXME. Do we want this?
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface() w=%d, h=%d", mWidth, mHeight));

    custom_cursor_texture = 0;

    mZBuffer = NULL;

    mSceneBegun = false;

    lastDrawMode = Graphics::DRAWMODE_NORMAL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

D3DInterface::~D3DInterface()
{
    Cleanup();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::UpdateViewport()
{
    int ww = gSexyAppBase->mVideoModeWidth;
    int wh = gSexyAppBase->mVideoModeHeight;

    int gw = gSexyAppBase->mWidth;
    int gh = gSexyAppBase->mHeight;

    float game_aspectratio = (float)gSexyAppBase->mWidth / gSexyAppBase->mHeight;
    bool game_is_landscape = game_aspectratio > 1.0;
    float window_aspectratio = (float)ww / wh;
    bool window_is_landscape = window_aspectratio > 1.0;
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: wind asp ratio=%f", window_aspectratio));
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: game asp ratio=%f", game_aspectratio));

    // Do we need to rotate? And _should_ we rotate (i.e. do we have a mobile device)?
    bool do_rotate = false;      // assume we don't need to
    if (game_is_landscape != window_is_landscape) {
        do_rotate = true;
    }

    if (do_rotate) {
        // From here on width and height are swapped
        int tmp = wh;
        wh = ww;
        ww = tmp;
        window_aspectratio = (float)ww / wh;
        Logger::log(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: rotated asp ratio=%f", window_aspectratio));
    }

    int vx;
    int vy;
    int vw;
    int vh;
    if (game_aspectratio > window_aspectratio) {
        // game dimension is wider than the available window
        // shrink to fit window width
        vw = ww;
        vh = vw / game_aspectratio;
        vx = 0;
        vy = (wh - vh) / 2;
    }
    else if (game_aspectratio < window_aspectratio) {
        // game dimension is higher than the available window
        // shrink to fit window height
        vh = wh;
        vw = vh * game_aspectratio;
        vx = (ww - vw) / 2;
        vy = 0;
    }
    else {
        // game and window have same aspect ratio
        vw = ww;
        vh = wh;
        vx = 0;
        vy = 0;
    }
    gSexyAppBase->mViewportToGameRatio = (float)gw / vw;
    gSexyAppBase->mViewportIsRotated = do_rotate;
#ifdef DEBUG
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: viewport to game ratio: %f", gSexyAppBase->mViewportToGameRatio));
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (do_rotate) {
        // rotate right

        // Do we need to swap vw/vh?
        int tmp = vw;
        vw = vh;
        vh = tmp;
        tmp = vx;
        vx = vy;
        vy = tmp;

#ifdef USE_OPENGLES
        // OpenGLES has glOrthof (with float parms)
        glOrthof(0, gh, gw, 0, -1, 1);
#else
        // OpenGL has glOrtho (with double parms)
        glOrtho(0, gh, gw, 0, -1, 1);
#endif

        glRotatef(90.0, 0.0, 0.0, 1.0);
        glTranslatef(0.0, -gh, 0.0);
    }
    else {
#ifdef USE_OPENGLES
        // OpenGLES has glOrthof (with float parms)
        glOrthof(0, gw, gh, 0, -1, 1);
#else
        // OpenGL has glOrtho (with double parms)
        glOrtho(0, gw, gh, 0, -1, 1);
#endif
    }

#if 1
    {
        //vx = 0;
        //vy = 0;
    }
#endif

#ifdef DEBUG
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: viewport x=%d, y=%d w=%d h=%d", vx, vy, vw, vh));
#endif
    gSexyAppBase->mViewportx = vx;
    gSexyAppBase->mViewporty = vy;
    gSexyAppBase->mViewportWidth = vw;
    gSexyAppBase->mViewportHeight = vh;
    glViewport(vx, vy, vw, vh);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::InitD3D()
{

#if 0
    GLint minimum_width = 1;
    GLint minimum_height = 1;

    while (true) {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, minimum_width, minimum_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        GLint width = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        GLint height = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        if (width == 0)
            minimum_width <<= 1;

        if (height == 0)
            minimum_height <<= 1;

        if (width != 0 && height != 0)
            break;
    }
#endif

    //GLint try_width = minimum_width;
    //GLint try_height = minimum_height;

    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

#if 0
    while (true) {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, try_width << 1, try_height << 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        GLint width = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_MAX_TEXTURE_SIZE, &width);
        GLint height = 0;
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        if (width != 0)
            try_width <<= 1;

        if (height != 0)
            try_height <<= 1;

        if (width == 0 && height == 0)
            break;
    }
#endif

    TextureData::SetMaxTextureDimension(max_texture_size, max_texture_size);
    //FIXME
    TextureData::SetMaxTextureAspectRatio(1);

#if 0
    if (gMinTextureWidth > gMaxTextureWidth)
        gMinTextureWidth = 64;
    if (gMinTextureHeight > gMaxTextureHeight)
        gMinTextureHeight = 64;
#endif

#if 0
    gSupportedPixelFormats = 0;
    mD3DDevice->EnumTextureFormats(PixelFormatsCallback, NULL);

    if (!(aCaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)) // need alpha in palettes
        gSupportedPixelFormats &= ~PixelFormat_Palette8;
#endif

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth (2.5);
    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_NORMALIZE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FOG);
    glShadeModel(GL_FLAT);

//   glReadBuffer(GL_BACK);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

//   glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
//   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glClear(GL_COLOR_BUFFER_BIT);
//   glDisable(GL_TEXTURE_GEN_S);
//   glDisable(GL_TEXTURE_GEN_T);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    UpdateViewport();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //create texture for mOldCursorArea
    glGenTextures(1, &custom_cursor_texture);
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unsigned char tmp[64 * 64 * 4];
    memset(tmp, 0, 64 * 64 * 4);
    glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA,
            64, 64,
            0,
            GL_RGBA,                    // We're only writing zeroes, so it doesn't matter
            GL_UNSIGNED_BYTE,
            tmp);

    gLinearFilter = false;
    SetLinearFilter(true);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::InitFromDDInterface(DDInterface *theInterface)
{
    mErrorString.erase();
#if 0
    mDD = theInterface->mDD7;
    mHWnd = theInterface->mHWnd;
#endif
    mWidth = theInterface->mWidth;
    mHeight = theInterface->mHeight;
    Logger::log(mLogFacil, 1, Logger::format("D3DInterface::InitFromDDInterface w=%d, h=%d", mWidth, mHeight));

    return InitD3D();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool gD3DInterfacePreDrawError = false;

bool D3DInterface::PreDraw()
{
    if (gSexyAppBase->mPhysMinimized)
        return false;

    if (!mSceneBegun) {
        gD3DInterfacePreDrawError = false;

#if 0
        hr = mD3DDevice->BeginScene();
#endif    

#if 0
        // alphablend states
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

        // filter states
        mD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFG_POINT);
        mD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        mD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        hr = mD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

        // Setup non-texture render states
        mD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
        mD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
        hr = mD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
#endif
        mSceneBegun = true;
        //gLinearFilter = false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
struct PointClipper {
    Pred mPred;

    void ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList & out);
    void ClipPoints(int n, float clipVal, VertexList &in, VertexList & out);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
void PointClipper<Pred>::ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList &out)
{
    if (!mPred(GetCoord(v1, n), clipVal)) {
        if (!mPred(GetCoord(v2, n), clipVal)) // both inside
            out.push_back(v2);
        else // inside -> outside
        {
            float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
            out.push_back(Interpolate(v1, v2, t));
        }
    } else {
        if (!mPred(GetCoord(v2, n), clipVal)) // outside -> inside
        {
            float t = (clipVal - GetCoord(v1, n)) / (GetCoord(v2, n) - GetCoord(v1, n));
            out.push_back(Interpolate(v1, v2, t));
            out.push_back(v2);
        }
        //      else // outside -> outside
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Pred>
void PointClipper<Pred>::ClipPoints(int n, float clipVal, VertexList &in, VertexList &out)
{
    if (in.size() < 2)
        return;

    ClipPoint(n, clipVal, in[in.size() - 1], in[0], out);
    for (VertexList::size_type i = 0; i < in.size() - 1; i++)
        ClipPoint(n, clipVal, in[i], in[i + 1], out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DrawPolyClipped(const Rect *theClipRect, const VertexList &theList)
{
    VertexList l1, l2;
    l1 = theList;

    int left = theClipRect->mX;
    int right = left + theClipRect->mWidth;
    int top = theClipRect->mY;
    int bottom = top + theClipRect->mHeight;

    VertexList *in = &l1, *out = &l2;
    PointClipper<std::less<float> > aLessClipper;
    PointClipper<std::greater_equal<float> > aGreaterClipper;

    aLessClipper.ClipPoints(0, left, *in, *out);
    std::swap(in, out);
    out->clear();
    aLessClipper.ClipPoints(1, top, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(0, right, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(1, bottom, *in, *out);

    VertexList &aList = *out;

    if (aList.size() >= 3) {
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].tu));
        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void DoPolyTextureClip(VertexList &theList)
{
    VertexList l2;

    float left = 0;
    float right = 1;
    float top = 0;
    float bottom = 1;

    VertexList *in = &theList, *out = &l2;
    PointClipper<std::less<float> > aLessClipper;
    PointClipper<std::greater_equal<float> > aGreaterClipper;

    aLessClipper.ClipPoints(3, left, *in, *out);
    std::swap(in, out);
    out->clear();
    aLessClipper.ClipPoints(4, top, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(3, right, *in, *out);
    std::swap(in, out);
    out->clear();
    aGreaterClipper.ClipPoints(4, bottom, *in, *out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::BltTransformed(const SexyMatrix3 &theTrans, const Rect& theSrcRect, const Color& theColor, const Rect *theClipRect, float theX, float theY, bool center)
{
    int srcLeft = theSrcRect.mX;
    int srcTop = theSrcRect.mY;
    int srcRight = srcLeft + theSrcRect.mWidth;
    int srcBottom = srcTop + theSrcRect.mHeight;
    int srcX, srcY;
    float dstX, dstY;
    int aWidth;
    int aHeight;
    float u1, v1, u2, v2;
    float startx = 0, starty = 0;
    float pixelcorrect = 0.0f; //0.5f;

    if (center) {
        startx = -theSrcRect.mWidth / 2.0f;
        starty = -theSrcRect.mHeight / 2.0f;
        pixelcorrect = 0.0f;
    }

    srcY = srcTop;
    dstY = starty;

    if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
        return;

    SexyRGBA rgba = theColor.ToRGBA();

    while (srcY < srcBottom) {
        srcX = srcLeft;
        dstX = startx;
        while (srcX < srcRight) {
            aWidth = srcRight - srcX;
            aHeight = srcBottom - srcY;
            GLuint aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

            float x = dstX; // - pixelcorrect; // - 0.5f; //FIXME correct??
            float y = dstY; // - pixelcorrect; // - 0.5f;

            SexyVector2 p[4] = {SexyVector2(x, y), SexyVector2(x, y + aHeight), SexyVector2(x + aWidth, y), SexyVector2(x + aWidth, y + aHeight)};
            SexyVector2 tp[4];

            int i;
            for (i = 0; i < 4; i++) {
                tp[i] = theTrans * p[i];
                tp[i].x -= pixelcorrect - theX;
                tp[i].y -= pixelcorrect - theY;
            }

            bool clipped = false;
            if (theClipRect != NULL) {
                int left = theClipRect->mX;
                int right = left + theClipRect->mWidth;
                int top = theClipRect->mY;
                int bottom = top + theClipRect->mHeight;
                for (i = 0; i < 4; i++) {
                    if (tp[i].x < left || tp[i].x >= right || tp[i].y < top || tp[i].y >= bottom) {
                        clipped = true;
                        break;
                    }
                }
            }

            glBindTexture(GL_TEXTURE_2D, aTexture);

            if (!clipped) {
                D3DTLVERTEX aVertex[4] =
                {
                    { u1,v1,rgba, tp[0].x,          tp[0].y,            0},
                    { u1,v2,rgba, tp[1].x,          tp[1].y,            0},
                    { u2,v1,rgba, tp[2].x,          tp[2].y,            0},
                    { u2,v2,rgba, tp[3].x,          tp[3].y,            0},
                };


                glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
                glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
                glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].tu));
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            } else {
                VertexList aList;

                D3DTLVERTEX vertex0 = {(GLfloat) u1, (GLfloat) v1, rgba, (GLfloat) tp[0].x, (GLfloat) tp[0].y};
                D3DTLVERTEX vertex1 = {(GLfloat) u1, (GLfloat) v2, rgba, (GLfloat) tp[1].x, (GLfloat) tp[1].y};
                D3DTLVERTEX vertex2 = {(GLfloat) u2, (GLfloat) v1, rgba, (GLfloat) tp[2].x, (GLfloat) tp[2].y};
                D3DTLVERTEX vertex3 = {(GLfloat) u2, (GLfloat) v2, rgba, (GLfloat) tp[3].x, (GLfloat) tp[3].y};

                aList.push_back(vertex0);
                aList.push_back(vertex1);
                aList.push_back(vertex3);
                aList.push_back(vertex2);

                DrawPolyClipped(theClipRect, aList);
            }
            srcX += aWidth;
            dstX += aWidth;

        }
        srcY += aHeight;
        dstY += aHeight;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color ? theVertex.color : theColor)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, Uint32 theColor, float tx, float ty)
{
    if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0)) {
        glBindTexture(GL_TEXTURE_2D, mTextures[0].mTexture);

        D3DTLVERTEX aVertexCache[300];
        int aVertexCacheNum = 0;

        glTexCoordPointer(2, GL_FLOAT, sizeof (D3DTLVERTEX), aVertexCache);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof (D3DTLVERTEX), &(aVertexCache[0].color));
        glVertexPointer(3, GL_FLOAT, sizeof (D3DTLVERTEX), &(aVertexCache[0].sx));

        for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++) {
            Color col;
            TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];
            D3DTLVERTEX* aD3DVertex = &aVertexCache[aVertexCacheNum];
            aVertexCacheNum += 3;

            aD3DVertex[0].sx = aTriVerts[0].x + tx;
            aD3DVertex[0].sy = aTriVerts[0].y + ty;
            aD3DVertex[0].sz = 0;
            //    aD3DVertex[0].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[0].color = col.ToRGBA();
            //aD3DVertex[0].specular = 0;
            aD3DVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
            aD3DVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

            aD3DVertex[1].sx = aTriVerts[1].x + tx;
            aD3DVertex[1].sy = aTriVerts[1].y + ty;
            aD3DVertex[1].sz = 0;
            //aD3DVertex[1].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[1].color = col.ToRGBA();
            //aD3DVertex[1].specular = 0;
            aD3DVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
            aD3DVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

            aD3DVertex[2].sx = aTriVerts[2].x + tx;
            aD3DVertex[2].sy = aTriVerts[2].y + ty;
            aD3DVertex[2].sz = 0;
            //aD3DVertex[2].rhw = 1;
            col = GetColorFromTriVertex(aTriVerts[0], theColor);
            aD3DVertex[2].color = col.ToRGBA();
            //aD3DVertex[2].specular = 0;
            aD3DVertex[2].tu = aTriVerts[2].u * mMaxTotalU;
            aD3DVertex[2].tv = aTriVerts[2].v * mMaxTotalV;

            if ((aVertexCacheNum == 300) || (aTriangleNum == theNumTriangles - 1)) {


                glDrawArrays(GL_TRIANGLES, 0, aVertexCacheNum);
                aVertexCacheNum = 0;
            }
        }
    } else {
        for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++) {
            TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];

            D3DTLVERTEX aVertex[3];
            Color col = GetColorFromTriVertex(aTriVerts[0], theColor);

            D3DTLVERTEX vertex1 = {(GLfloat) (aTriVerts[0].u * mMaxTotalU), (GLfloat) (aTriVerts[0].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[0].x + tx, aTriVerts[0].u + ty, 0.0f};

            col = GetColorFromTriVertex(aTriVerts[1], theColor);

            D3DTLVERTEX vertex2 = {(GLfloat) (aTriVerts[1].u * mMaxTotalU), (GLfloat) (aTriVerts[1].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[1].x + tx, aTriVerts[1].u + ty, 0.0f};
            col = GetColorFromTriVertex(aTriVerts[2], theColor);

            D3DTLVERTEX vertex3 = {(GLfloat) (aTriVerts[2].u * mMaxTotalU), (GLfloat) (aTriVerts[2].v * mMaxTotalV),
                col.ToRGBA(),
                aTriVerts[2].x + tx, aTriVerts[2].u + ty, 0.0f};

            aVertex[0] = vertex1;
            aVertex[1] = vertex2;
            aVertex[2] = vertex3;

            float aMinU = mMaxTotalU, aMinV = mMaxTotalV;
            float aMaxU = 0, aMaxV = 0;

            int i, j, k;
            for (i = 0; i < 3; i++) {
                if (aVertex[i].tu < aMinU)
                    aMinU = aVertex[i].tu;

                if (aVertex[i].tv < aMinV)
                    aMinV = aVertex[i].tv;

                if (aVertex[i].tu > aMaxU)
                    aMaxU = aVertex[i].tu;

                if (aVertex[i].tv > aMaxV)
                    aMaxV = aVertex[i].tv;
            }

            VertexList aMasterList;
            aMasterList.push_back(aVertex[0]);
            aMasterList.push_back(aVertex[1]);
            aMasterList.push_back(aVertex[2]);

            VertexList aList;

            int aLeft = (int) floorf(aMinU);
            int aTop = (int) floorf(aMinV);
            int aRight = (int) ceilf(aMaxU);
            int aBottom = (int) ceilf(aMaxV);
            if (aLeft < 0)
                aLeft = 0;
            if (aTop < 0)
                aTop = 0;
            if (aRight > mTexVecWidth)
                aRight = mTexVecWidth;
            if (aBottom > mTexVecHeight)
                aBottom = mTexVecHeight;

            TextureDataPiece &aStandardPiece = mTextures[0];
            for (i = aTop; i < aBottom; i++) {
                for (j = aLeft; j < aRight; j++) {
                    TextureDataPiece &aPiece = mTextures[i * mTexVecWidth + j];


                    VertexList aList = aMasterList;
                    for (k = 0; k < 3; k++) {
                        aList[k].tu -= j;
                        aList[k].tv -= i;
                        if (i == mTexVecHeight - 1)
                            aList[k].tv *= (float) aStandardPiece.mHeight / aPiece.mHeight;
                        if (j == mTexVecWidth - 1)
                            aList[k].tu *= (float) aStandardPiece.mWidth / aPiece.mWidth;
                    }

                    DoPolyTextureClip(aList);
                    if (aList.size() >= 3) {
                        glBindTexture(GL_TEXTURE_2D, aPiece.mTexture);
                        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
                        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
                        glTexCoordPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].tu));
                        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::CreateImageTexture(MemoryImage *theImage)
{
    bool wantPurge = false;

    if (theImage->mD3DData == NULL) {
        theImage->mD3DData = new TextureData();

        // The actual purging was deferred
        wantPurge = theImage->mPurgeBits;

#if 0
        AutoCrit aCrit(gSexyAppBase->mDDInterface->mCritSect); // Make images thread safe
#endif
        mImageSet.insert(theImage);
    }

    TextureData *aData = (TextureData*) theImage->mD3DData;
    aData->CheckCreateTextures(theImage);

    if (wantPurge)
        theImage->PurgeBits();

    //FIXME
    return true; //aData->mPixelFormat != PixelFormat_Unknown;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::RecoverBits(MemoryImage* theImage)
{
#if 0
    if (theImage->mD3DData == NULL)
        return false;

    TextureData* aData = (TextureData*) theImage->mD3DData;
    if (aData->mBitsChangedCount != theImage->mBitsChangedCount) // bits have changed since texture was created
        return false;

    for (int aPieceRow = 0; aPieceRow < aData->mTexVecHeight; aPieceRow++) {
        for (int aPieceCol = 0; aPieceCol < aData->mTexVecWidth; aPieceCol++) {
            TextureDataPiece* aPiece = &aData->mTextures[aPieceRow * aData->mTexVecWidth + aPieceCol];

            int offx = aPieceCol * aData->mTexPieceWidth;
            int offy = aPieceRow * aData->mTexPieceHeight;
            int aWidth = std::min(theImage->GetWidth() - offx, aPiece->mWidth);
            int aHeight = std::min(theImage->GetHeight() - offy, aPiece->mHeight);

            CopySurface8888ToImage(aPiece->mTexture, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_A4R4G4B4:
            CopyTexture4444ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_R5G6B5:
            CopyTexture565ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight);
            break;
        case PixelFormat_Palette8:
            CopyTexturePalette8ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight, aData->mPalette);
            break;
        }


    }

#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::SetCurTexture(MemoryImage *theImage)
{
    if (theImage == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    if (!CreateImageTexture(theImage))
        return;

    TextureData *aData = (TextureData*) theImage->mD3DData;

    glBindTexture(GL_TEXTURE_2D, aData->mTextures[0].mTexture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
    if (mTransformStack.empty() || !concatenate)
        mTransformStack.push_back(theTransform);
    else {
        SexyMatrix3 &aTrans = mTransformStack.back();
        mTransformStack.push_back(theTransform * aTrans);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::PopTransform()
{
    if (!mTransformStack.empty())
        mTransformStack.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::RemoveMemoryImage(MemoryImage *theImage)
{
    if (theImage->mD3DData != NULL) {
        delete (TextureData*) theImage->mD3DData;
        theImage->mD3DData = NULL;
#if 0
        AutoCrit aCrit(gSexyAppBase->mDDInterface->mCritSect); // Make images thread safe
#endif
        mImageSet.erase(theImage);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Cleanup()
{
    Flush();

    ImageSet::iterator anItr;
    for (anItr = mImageSet.begin(); anItr != mImageSet.end(); ++anItr) {
        MemoryImage *anImage = *anItr;
        delete (TextureData*) anImage->mD3DData;
        anImage->mD3DData = NULL;
    }

    mImageSet.clear();

#if 0
    if (mD3DDevice != NULL) {
        mD3DDevice->Release();
        mD3DDevice = NULL;
    }

    if (mD3D != NULL) {
        mD3D->Release();
        mD3D = NULL;
    }
#endif

    if (mZBuffer != NULL) {
        SDL_FreeSurface(mZBuffer);
        mZBuffer = NULL;
    }

    //if (glIsTexture(custom_cursor_texture) == GL_TRUE)
    //    glDeleteTextures(1, &custom_cursor_texture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::SetupDrawMode(int theDrawMode, const Color &theColor, Image *theImage)
{
    if (lastDrawMode == theDrawMode)
        return;
    if (theDrawMode == Graphics::DRAWMODE_NORMAL) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else // Additive
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    lastDrawMode = theDrawMode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
    if (!mTransformStack.empty()) {
        BltClipF(theImage, theX, theY, theSrcRect, NULL, theColor, theDrawMode);
        return;
    }

    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(linearFilter);
    //SetLinearFilter(true);

    aData->Blt(theX, theY, theSrcRect, theColor);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
    SexyTransform2D aTransform;

    aTransform.Translate(-theSrcRect.mWidth, 0);
    aTransform.Scale(-1, 1);
    aTransform.Translate(theX, theY);

    BltTransformed(theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform, linearFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode)
{
    SexyTransform2D aTransform;
    aTransform.Translate(theX, theY);

    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
    float xScale = (float) theDestRect.mWidth / theSrcRect.mWidth;
    float yScale = (float) theDestRect.mHeight / theSrcRect.mHeight;

    SexyTransform2D aTransform;
    if (mirror) {
        aTransform.Translate(-theSrcRect.mWidth, 0);
        aTransform.Scale(-xScale, yScale);
    } else
        aTransform.Scale(xScale, yScale);

    aTransform.Translate(theDestRect.mX, theDestRect.mY);
    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, !fastStretch);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect &theSrcRect)
{
    SexyTransform2D aTransform;

    aTransform.Translate(-theRotCenterX, -theRotCenterY);
    aTransform.RotateRad(theRot);
    aTransform.Translate(theX + theRotCenterX, theY + theRotCenterY);

    BltTransformed(theImage, theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theImage);

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(true); // force linear filtering in the case of a global transform

    if (!mTransformStack.empty()) {

        if (theX != 0 || theY != 0) {
            SexyTransform2D aTransform;
            if (center)
                aTransform.Translate(-theSrcRect.mWidth / 2.0f, -theSrcRect.mHeight / 2.0f);

            aTransform = theTransform * aTransform;
            aTransform.Translate(theX, theY);
            aTransform = mTransformStack.back() * aTransform;

            aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect);
        } else {
            SexyTransform2D aTransform = mTransformStack.back() * theTransform;
            aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
        }
    } else {
        //SetLinearFilter(linearFilter);

        aData->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);

    float x1, y1, x2, y2;
    SexyRGBA aColor = theColor.ToRGBA();

    if (!mTransformStack.empty()) {
        SexyVector2 p1(theStartX, theStartY);
        SexyVector2 p2(theEndX, theEndY);
        p1 = mTransformStack.back() * p1;
        p2 = mTransformStack.back() * p2;

        x1 = p1.x;
        y1 = p1.y;
        x2 = p2.x;
        y2 = p2.y;
    } else {
        x1 = theStartX;
        y1 = theStartY;
        x2 = theEndX;
        y2 = theEndY;
    }

    D3DTLVERTEX aVertex[2] =
    {
        { 0, 0, aColor, x1, y1, 0},
        { 0, 0, aColor, x2, y2, 0}
    };

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_LINE_STRIP, 0, 2);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);

    SexyRGBA aColor = theColor.ToRGBA();

    float x = theRect.mX; // - 0.5f;
    float y = theRect.mY; // - 0.5f;
    float aWidth = theRect.mWidth;
    float aHeight = theRect.mHeight;

    D3DTLVERTEX aVertex[4] ={
        { 0, 0, aColor, x,          y,           0},
        { 0, 0, aColor, x,          y + aHeight, 0},
        { 0, 0, aColor, x + aWidth, y,           0},
        { 0, 0, aColor, x + aWidth, y + aHeight, 0}
    };

    if (!mTransformStack.empty()) {
        SexyVector2 p[4] = {SexyVector2(x, y), SexyVector2(x, y + aHeight), SexyVector2(x + aWidth, y), SexyVector2(x + aWidth, y + aHeight)};

        int i;
        for (i = 0; i < 4; i++) {
            p[i] = mTransformStack.back() * p[i];
            //p[i].x -= 0.5f;
            //p[i].y -= 0.5f;
            aVertex[i].sx = p[i].x;
            aVertex[i].sy = p[i].y;
        }
    }

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode)
{
    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);
    Color aColor1 = GetColorFromTriVertex(p1, theColor);
    Color aColor2 = GetColorFromTriVertex(p2, theColor);
    Color aColor3 = GetColorFromTriVertex(p3, theColor);

    SexyRGBA aRGBA1 = aColor1.ToRGBA();
    SexyRGBA aRGBA2 = aColor2.ToRGBA();
    SexyRGBA aRGBA3 = aColor3.ToRGBA();

    D3DTLVERTEX aVertex[3] ={
        { 0, 0, aRGBA1, p1.x, p1.y, 0},
        { 0, 0, aRGBA2, p2.x, p2.y, 0},
        { 0, 0, aRGBA3, p3.x, p3.y, 0}
    };


    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
    if (theNumVertices < 3)
        return;

    if (!PreDraw())
        return;

    SetupDrawMode(theDrawMode, theColor, NULL);
    SexyRGBA aColor = theColor.ToRGBA();

    VertexList aList;
    for (int i = 0; i < theNumVertices; i++) {
        D3DTLVERTEX vert = {0, 0, aColor, theVertices[i].mX + tx, theVertices[i].mY + ty, 0};
        if (!mTransformStack.empty()) {
            SexyVector2 v(vert.sx, vert.sy);
            v = mTransformStack.back() * v;
            vert.sx = v.x;
            vert.sy = v.y;
        }

        aList.push_back(vert);
    }

    if (theClipRect != NULL)
        DrawPolyClipped(theClipRect, aList);
    else {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
        glVertexPointer(2, GL_FLOAT, sizeof(D3DTLVERTEX), &(aList[0].sx));
        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());

        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend)
{
    TriVertex aVertices[1][3] = {
        {p1, p2, p3}};
    DrawTrianglesTex(aVertices, 1, theColor, theDrawMode, theTexture, blend);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
    if (!PreDraw())
        return;

    MemoryImage* aSrcMemoryImage = dynamic_cast<MemoryImage*>(theTexture);

    if (!CreateImageTexture(aSrcMemoryImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theTexture);

    TextureData *aData = (TextureData*) aSrcMemoryImage->mD3DData;

    //SetLinearFilter(blend);
    //SetLinearFilter(true);

    aData->BltTriangles(theVertices, theNumTriangles, (Uint32) theColor.ToInt(), tx, ty);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
    TriVertex aList[100][3];
    int aTriNum = 0;
    while (aTriNum < theNumTriangles) {
        int aMaxTriangles = std::min(100, theNumTriangles - aTriNum);
        for (int i = 0; i < aMaxTriangles; i++) {
            aList[i][0] = theVertices[aTriNum];
            aList[i][1] = theVertices[aTriNum + 1];
            aList[i][2] = theVertices[aTriNum + 2];
            aTriNum++;
        }
        DrawTrianglesTex(aList, aMaxTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::Flush()
{
    if (mSceneBegun) {
#if 0
        mD3DDevice->EndScene();
#endif
        mSceneBegun = false;
        mErrorString.erase();
    }
}

bool D3DInterface::glIsExtensionSupported(const char *extension)
{
  const GLubyte *extensions = NULL;
  const GLubyte *start;

  GLubyte *where, *terminator;
  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');

  if (where || *extension == '\0')
    return false;

  extensions = glGetString(GL_EXTENSIONS);

  /* It takes a bit of care to be fool-proof about parsing the

     OpenGL extensions string. Don't be fooled by sub-strings,

     etc. */
  start = extensions;

  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if (where == start || *(where - 1) == ' ')

      if (*terminator == ' ' || *terminator == '\0')
        return true;

    start = terminator;
  }
  return false;
}
