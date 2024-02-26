#include "D3DInterface.h"
#include "TextureData.h"
#include "VertexList.h"
#include "DDInterface.h"
#include "Graphics.h"
#include "Color.h"
#include "GLExtensions.h"
#include "GLState.h"
#include "Common.h"
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

D3DInterface::D3DInterface()
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("d3dinterface");
    TLOG(mLogFacil, 1, "new D3DInterface");
#endif

    mWidth = 640;
    mHeight = 480;
    LOG(mLogFacil, 1, Logger::format("D3DInterface() w=%d, h=%d", mWidth, mHeight));

    custom_cursor_texture = 0;

    mZBuffer = NULL;

    mSceneBegun = false;

    lastDrawMode = Graphics::DRAWMODE_NONE;
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
    LOG(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: wind asp ratio=%f", window_aspectratio));
    LOG(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: game asp ratio=%f", game_aspectratio));

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
        LOG(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: rotated asp ratio=%f", window_aspectratio));
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
    LOG(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: viewport to game ratio: %f", gSexyAppBase->mViewportToGameRatio));

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

    LOG(mLogFacil, 1, Logger::format("D3DInterface::UpdateViewport: viewport x=%d, y=%d w=%d h=%d", vx, vy, vw, vh));
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
    if (!GLExtensions::glEnableVertexBufferObjects())
        assert(false);

    //GLint try_width = minimum_width;
    //GLint try_height = minimum_height;

    GLint max_texture_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

    TextureData::SetMaxTextureDimension(max_texture_size, max_texture_size);
    //FIXME
    TextureData::SetMaxTextureAspectRatio(1);

#if TARGET_OS_IPHONE
    glDisable(GL_LINE_SMOOTH);
#else
    glEnable(GL_LINE_SMOOTH);
#endif
    GLState::getInstance()->disable(GL_BLEND);
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
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 0.0);

//   glReadBuffer(GL_BACK);
//   glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
//   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
//   glDisable(GL_TEXTURE_GEN_S);
//   glDisable(GL_TEXTURE_GEN_T);

    UpdateViewport();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0f / TEXTURESCALING, 1.0f / TEXTURESCALING, 1.0f / TEXTURESCALING);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //create texture for mOldCursorArea
    glGenTextures(1, &custom_cursor_texture);
    GLState::getInstance()->bindTexture(GL_TEXTURE_2D, custom_cursor_texture);
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
    mWidth = theInterface->mWidth;
    mHeight = theInterface->mHeight;
    LOG(mLogFacil, 1, Logger::format("D3DInterface::InitFromDDInterface w=%d, h=%d", mWidth, mHeight));

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

        mSceneBegun = true;
        //gLinearFilter = false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool D3DInterface::CreateImageTexture(Image *theImage)
{
    bool wantPurge = false;

    if (!theImage->HasTextureData()) {
        theImage->CreateTextureData();

        // The actual purging was deferred
        wantPurge = theImage->GetPurgeBits();

        // FIXME. Why do we only register images with new TextureData?
        mImageSet.insert(theImage);
    }

    theImage->CheckCreateTextures();

    if (wantPurge)
        theImage->DoPurgeBits();

    //FIXME
    return true; //aData->mPixelFormat != PixelFormat_Unknown;
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

void D3DInterface::RemoveImage(Image *theImage)
{
    if (theImage->HasTextureData()) {
        theImage->DeleteTextureData();
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
        Image *anImage = *anItr;
        anImage->DeleteTextureData();
    }

    mImageSet.clear();

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
        // Same as BltClipF, but using NULL pointer for cliprect
        SexyTransform2D aTransform;
        aTransform.Translate(theX, theY);

        BltTransformed(theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform, true);
        return;
    }

    if (!PreDraw())
        return;

    if (!CreateImageTexture(theImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = theImage->GetTextureData();

    //SetLinearFilter(linearFilter);
    //SetLinearFilter(true);

    if(aData->hasAlpha())
        GLState::getInstance()->enable(GL_BLEND);
    else
        GLState::getInstance()->disable(GL_BLEND);

    GLState::getInstance()->enable(GL_TEXTURE_2D);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
    GLState::getInstance()->enableClientState(GL_TEXTURE_COORD_ARRAY);

    if (theSrcRect.mX != 0 || theSrcRect.mY != 0 || theSrcRect.mWidth != theImage->GetWidth() || theSrcRect.mHeight != theImage->GetHeight()) {
        aData->Blt(theX, theY, theSrcRect, theColor);
    }
    else {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(theX, theY, 0.0f);

        if (theColor != Color::White)
            aData->Blt(theColor);
        else
            aData->Blt();

        glPopMatrix();
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
    //FIXME remove
    SexyTransform2D aTransform;

    glPushMatrix();
    glLoadIdentity();

    glTranslatef(theX, theY, 0.0f);
    glScalef(-1.0f, 1.0f, 0.0f);
    glTranslatef(-theSrcRect.mWidth, 0.0f, 0.0f);

    BltTransformed(theImage, NULL, theColor, theDrawMode, theSrcRect, aTransform, true);

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode)
{
    SexyTransform2D aTransform;
    aTransform.Translate(theX, theY);

    BltTransformed(theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::StretchBlt(Image* theImage, const Rect& theDestRect, const Rect& theSrcRect, const Rect& theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
    float xScale = (float) theDestRect.mWidth / theSrcRect.mWidth;
    float yScale = (float) theDestRect.mHeight / theSrcRect.mHeight;

    //FIXME remove
    SexyTransform2D aTransform;

    glPushMatrix();
    glLoadIdentity();

    glTranslatef(theDestRect.mX, theDestRect.mY, 0.0f);
    if (mirror) {
        glScalef(-xScale, yScale, 0.0f);
        glTranslatef(-theSrcRect.mWidth, 0.0f, 0.0f);
    }
    else {
        glScalef(xScale, yScale, 0.0f);
    }

    BltTransformed(theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform, !fastStretch);

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//theRot is in radians
void D3DInterface::BltRotated(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect& theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY)
{
    //FIXME remove this variable
    SexyTransform2D aTransform;

    glPushMatrix();
    glLoadIdentity();

    //opengl wants the angles in degrees not radians
    glTranslatef(theX + theRotCenterX, theY + theRotCenterY, 0.0f);
    glRotatef(radtodeg(theRot), 0.0f, 0.0f, -1.0f);
    glTranslatef(-theRotCenterX, -theRotCenterY, 0.0f);

    BltTransformed(theImage, &theClipRect, theColor, theDrawMode, theSrcRect, aTransform, true);

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void D3DInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
    if (!PreDraw())
        return;

    if (!CreateImageTexture(theImage))
        return;

    SetupDrawMode(theDrawMode, theColor, theImage);

    TextureData *aData = theImage->GetTextureData();

    if(aData->hasAlpha())
        GLState::getInstance()->enable(GL_BLEND);
    else
        GLState::getInstance()->disable(GL_BLEND);

    GLState::getInstance()->enable(GL_TEXTURE_2D);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_TEXTURE_COORD_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
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
    }
    else { /*mTransformStack.empty()*/
        if (center || theClipRect != NULL || theSrcRect.mX != 0 || theSrcRect.mY != 0 || theSrcRect.mWidth != theImage->GetWidth() || theSrcRect.mHeight != theImage->GetHeight()) {
            aData->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
        }
        else {
            if (theColor != Color::White)
                aData->BltTransformed(theColor);
            else
                aData->BltTransformed();
        }
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
        { x1,y1, aColor, 0, 0},
        { x2,y2, aColor, 0, 0}
    };

    GLState::getInstance()->disable(GL_TEXTURE_2D);
    GLState::getInstance()->disableClientState(GL_TEXTURE_COORD_ARRAY);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_SHORT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_LINE_STRIP, 0, 2);
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
        { x,y, aColor, 0, 0           },
        { x, y + aHeight, aColor, 0,0        },
        { x + aWidth, y, aColor, 0,0           },
        { x + aWidth, y + aHeight, aColor, 0,0 }
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

    GLState::getInstance()->disable(GL_TEXTURE_2D);
    GLState::getInstance()->disableClientState(GL_TEXTURE_COORD_ARRAY);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_SHORT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color ? theVertex.color : theColor)

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
        { p1.x, p1.y, aRGBA1, 0, 0},
        { p2.x, p2.y, aRGBA2, 0, 0},
        { p3.x, p3.y, aRGBA3, 0, 0}
    };


    GLState::getInstance()->disable(GL_TEXTURE_2D);
    GLState::getInstance()->disableClientState(GL_TEXTURE_COORD_ARRAY);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aVertex[0].color));
    glVertexPointer(2, GL_SHORT, sizeof(D3DTLVERTEX), &(aVertex[0].sx));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
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
        D3DTLVERTEX vert = {theVertices[i].mX + tx, theVertices[i].mY + ty, aColor, 0, 0};
        if (!mTransformStack.empty()) {
            SexyVector2 v(vert.sx, vert.sy);
            v = mTransformStack.back() * v;
            vert.sx = v.x;
            vert.sy = v.y;
        }

        aList.push_back(vert);
    }

    if (theClipRect != NULL)
        aList.DrawPolyClipped(theClipRect);
    else {
        GLState::getInstance()->disable(GL_TEXTURE_2D);
        GLState::getInstance()->disableClientState(GL_TEXTURE_COORD_ARRAY);
        GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
        GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(D3DTLVERTEX), &(aList[0].color));
        glVertexPointer(2, GL_SHORT, sizeof(D3DTLVERTEX), &(aList[0].sx));
        glDrawArrays(GL_TRIANGLE_FAN, 0, aList.size());
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

    if (!CreateImageTexture(theTexture))
        return;

    SetupDrawMode(theDrawMode, theColor, theTexture);

    TextureData *aData = theTexture->GetTextureData();

    //SetLinearFilter(blend);
    //SetLinearFilter(true);

    if (blend)
        GLState::getInstance()->enable(GL_BLEND);
    else
        GLState::getInstance()->disable(GL_BLEND);

    GLState::getInstance()->enable(GL_TEXTURE_2D);
    GLState::getInstance()->enableClientState(GL_TEXTURE_COORD_ARRAY);
    GLState::getInstance()->enableClientState(GL_VERTEX_ARRAY);
    GLState::getInstance()->enableClientState(GL_COLOR_ARRAY);

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
        mSceneBegun = false;
        mErrorString.erase();
    }
}
