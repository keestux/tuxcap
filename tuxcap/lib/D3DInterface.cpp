#include "D3DInterface.h"
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

//FIXME optimize, doing lot's of redundant operations like from surface to image and back to texture copy etc etc.. See how custom mouse pointers are blitted

using namespace Sexy;

static int gMinTextureWidth;
static int gMinTextureHeight;
static int gMaxTextureWidth;
static int gMaxTextureHeight;
static int gMaxTextureAspectRatio;
static Uint32 gSupportedPixelFormats;
static bool gTextureSizeMustBePow2;
static const int MAX_TEXTURE_SIZE = 1024;
static bool gLinearFilter = false;


#ifndef WIN32
typedef struct {
  GLfloat  tu;
  GLfloat tv;
  SexyRGBA color;
  GLfloat sx;
  GLfloat sy;
  GLfloat sz;
} D3DTLVERTEX;
#endif

std::string D3DInterface::mErrorString;
#if 0
static const int gVertexType = D3DFVF_TLVERTEX;
#endif

static void SetLinearFilter(bool linearFilter) {
  if (gLinearFilter != linearFilter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearFilter ? GL_LINEAR:GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linearFilter ? GL_LINEAR:GL_NEAREST);  
    gLinearFilter = linearFilter;
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static SDL_Surface* CreateTextureSurface(int theWidth, int theHeight/*, PixelFormat theFormat*/)
{

	SDL_Surface* aSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, theWidth, theHeight, 32, SDL_rmask, SDL_gmask, SDL_bmask, SDL_amask);

	return aSurface;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int GetClosestPowerOf2Above(int theNum)
{
	int aPower2 = 1;
	while (aPower2 < theNum)
		aPower2<<=1;

	return aPower2;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool IsPowerOf2(int theNum)
{
	int aNumBits = 0;
	while (theNum>0)
	{
		aNumBits += theNum&1;
		theNum >>= 1;
	}

	return aNumBits==1;
}

/* taken from a post by Sam Lantinga, thanks Sam for this and for SDL :-)*/
static GLuint CreateTexture(SDL_Surface *surface) {

  GLuint texture;
    int w, h;
    SDL_Surface *image;
    SDL_Rect area;
    Uint32 saved_flags;
    Uint8  saved_alpha;

    /* Use the surface width and height expanded to powers of 2 */
    w = GetClosestPowerOf2Above(surface->w);
    h = GetClosestPowerOf2Above(surface->h);

    image = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            w, h,
            32,
            SDL_rmask,
            SDL_gmask,
            SDL_bmask,
            SDL_amask
               );

    if ( image == NULL ) {
        return 0;
    }

    /* Save the alpha blending attributes */
    saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
    saved_alpha = surface->format->alpha;
    if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
        SDL_SetAlpha(surface, 0, 0);
    }

    /* Copy the surface into the GL texture image */
    area.x = 0;
    area.y = 0;
    area.w = surface->w;
    area.h = surface->h;
    SDL_BlitSurface(surface, &area, image, &area);

    /* Restore the alpha blending attributes */
    if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
        SDL_SetAlpha(surface, saved_flags, saved_alpha);
    }

    /* Create an OpenGL texture for the image */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    glTexImage2D(GL_TEXTURE_2D,
             0,
             GL_RGBA,
             w, h,
             0,
             GL_RGBA,
             GL_UNSIGNED_BYTE,
             image->pixels);

    SDL_FreeSurface(image); /* Image is no longer needed */

    return texture;

}

void D3DInterface::FillOldCursorAreaTexture(GLint x, GLint y) {
    glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0,0,0,x,y,64,64);
}

void D3DInterface::BltOldCursorArea(GLfloat x, GLfloat y, const Color& theColor)
{
  glEnable(GL_TEXTURE_2D); 	

  SetLinearFilter(false);

  SexyRGBA rgba = theColor.ToRGBA();
  glColor4ub(rgba.r, rgba.g, rgba.b, rgba.a);	
                        
  glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(x,y);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(x,y + 64);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(x + 64,y + 64);
  glTexCoord2f(1.0f,1.0f);
  glVertex2f(x+64,y);
  glEnd();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToSurface8888(void *theDest, Uint32 theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, bool rightPad)
{
	if (theImage->mColorTable == NULL)
	{
		ulong *srcRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;
		char *dstRow = (char*)theDest;

		for(int y=0; y<theHeight; y++)
		{
			ulong *src = srcRow;
			ulong *dst = (ulong*)dstRow;
			for(int x=0; x<theWidth; x++)
			{
				*dst++ = *src++;
			}

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else // palette
	{
		uchar *srcRow = (uchar*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uchar *dstRow = (uchar*)theDest;
		ulong *palette = theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uchar *src = srcRow;
			ulong *dst = (ulong*)dstRow;
			for(int x=0; x<theWidth; x++)
				*dst++ = palette[*src++];

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopySurface8888ToImage(void *theDest, Uint32 theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight)
{		
	char *srcRow = (char*)theDest;
	ulong *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

	for(int y=0; y<theHeight; y++)
	{
		ulong *src = (ulong*)srcRow;
		ulong *dst = dstRow;
		
		for(int x=0; x<theWidth; x++)
			*dst++ = *src++;		

		dstRow += theImage->GetWidth();
		srcRow += theDestPitch;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToSurface(MemoryImage *theImage, SDL_Surface* surface, int offx, int offy, int texWidth, int texHeight)
{
	if (surface==NULL)
		return;

	int aWidth = std::min(texWidth,(theImage->GetWidth()-offx));
	int aHeight = std::min(texHeight,(theImage->GetHeight()-offy));

	bool rightPad = aWidth<texWidth;
	bool bottomPad = aHeight<texHeight;

	if(aWidth>0 && aHeight>0)
	{
          CopyImageToSurface8888((Uint32*)surface->pixels, surface->pitch, theImage, offx, offy, aWidth, aHeight, rightPad); 
        }
		
        if (bottomPad)
		{
			uchar *dstrow = ((uchar*)surface->pixels)+surface->pitch*aHeight;
			memcpy(dstrow,dstrow-surface->pitch,surface->pitch);
		}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static void GetBestTextureDimensions(int &theWidth, int &theHeight, bool isEdge, bool usePow2, Uint32 theImageFlags)
{
	if (theImageFlags & D3DImageFlag_Use64By64Subdivisions)
	{
		theWidth = theHeight = 64;
		return;
	}

	static int aGoodTextureSize[MAX_TEXTURE_SIZE];
	static bool haveInited = false;
	if (!haveInited)
	{
		haveInited = true;
		int i;
		int aPow2 = 1;
		for (i=0; i<MAX_TEXTURE_SIZE; i++)
		{
			if (i > aPow2)
				aPow2 <<= 1;

			int aGoodValue = aPow2;
			if ((aGoodValue - i ) > 64)
			{
				aGoodValue >>= 1;
				while (true)
				{
					int aLeftOver = i % aGoodValue;
					if (aLeftOver<64 || IsPowerOf2(aLeftOver))
						break;

					aGoodValue >>= 1;
				}
			}
			aGoodTextureSize[i] = aGoodValue;
		}
	}

	int aWidth = theWidth;
	int aHeight = theHeight;

	if (usePow2)
	{
		if (isEdge || (theImageFlags & D3DImageFlag_MinimizeNumSubdivisions))
		{
			aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : GetClosestPowerOf2Above(aWidth);
			aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : GetClosestPowerOf2Above(aHeight);
		}
		else
		{
			aWidth = aWidth >= gMaxTextureWidth ? gMaxTextureWidth : aGoodTextureSize[aWidth];
			aHeight = aHeight >= gMaxTextureHeight ? gMaxTextureHeight : aGoodTextureSize[aHeight];
		}
	}

	if (aWidth < gMinTextureWidth)
		aWidth = gMinTextureWidth;

	if (aHeight < gMinTextureHeight)
		aHeight = gMinTextureHeight;

	if (aWidth > aHeight)
	{
		while (aWidth > gMaxTextureAspectRatio*aHeight)
			aHeight <<= 1;
	}
	else if (aHeight > aWidth)
	{
		while (aHeight > gMaxTextureAspectRatio*aWidth)
			aWidth <<= 1;
	}

	theWidth = aWidth;
	theHeight = aHeight;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TextureData::TextureData()
{
	mWidth = 0;
	mHeight = 0;
	mTexVecWidth = 0;
	mTexVecHeight = 0;
	mBitsChangedCount = 0;
	mTexMemSize = 0;
	mTexPieceWidth = 64;
	mTexPieceHeight = 64;

#if 0
	mPalette = NULL;
#endif
	mPixelFormat = PixelFormat_Unknown;
	mImageFlags = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TextureData::~TextureData()
{
	ReleaseTextures();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::ReleaseTextures()
{
	for(int i=0; i<(int)mTextures.size(); i++)
	{
          glDeleteTextures(1, &mTextures[i].mTexture);
	}

	mTextures.clear();

	mTexMemSize = 0;
#if 0
	if (mPalette!=NULL)
	{
		mPalette->Release();
		mPalette = NULL;
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CreateTextureDimensions(MemoryImage *theImage)
{
	int aWidth = theImage->GetWidth();
	int aHeight = theImage->GetHeight();
	int i;
/**/
	// Calculate inner piece sizes
	mTexPieceWidth = aWidth;
	mTexPieceHeight = aHeight;
	bool usePow2 = true; //gTextureSizeMustBePow2 || mPixelFormat==PixelFormat_Palette8;
	GetBestTextureDimensions(mTexPieceWidth, mTexPieceHeight,false,usePow2,mImageFlags);

	// Calculate right boundary piece sizes
	int aRightWidth = aWidth%mTexPieceWidth;
	int aRightHeight = mTexPieceHeight;
	if (aRightWidth > 0)
		GetBestTextureDimensions(aRightWidth, aRightHeight,true,usePow2,mImageFlags);
	else
		aRightWidth = mTexPieceWidth;

	// Calculate bottom boundary piece sizes
	int aBottomWidth = mTexPieceWidth;
	int aBottomHeight = aHeight%mTexPieceHeight;
	if (aBottomHeight > 0)
		GetBestTextureDimensions(aBottomWidth, aBottomHeight,true,usePow2,mImageFlags);
	else
		aBottomHeight = mTexPieceHeight;

	// Calculate corner piece size
	int aCornerWidth = aRightWidth;
	int aCornerHeight = aBottomHeight;
	GetBestTextureDimensions(aCornerWidth, aCornerHeight,true,usePow2,mImageFlags);
/**/

	// Allocate texture array
	mTexVecWidth = (aWidth + mTexPieceWidth - 1)/mTexPieceWidth;
	mTexVecHeight = (aHeight + mTexPieceHeight - 1)/mTexPieceHeight;
	mTextures.resize(mTexVecWidth*mTexVecHeight);
	
	// Assign inner pieces
	for(i=0; i<(int)mTextures.size(); i++)
	{
		TextureDataPiece &aPiece = mTextures[i];
		aPiece.mTexture = 0;
		aPiece.mWidth = mTexPieceWidth;
		aPiece.mHeight = mTexPieceHeight;
	}

	// Assign right pieces
/**/
	for(i=mTexVecWidth-1; i<(int)mTextures.size(); i+=mTexVecWidth)
	{
		TextureDataPiece &aPiece = mTextures[i];
		aPiece.mWidth = aRightWidth;
		aPiece.mHeight = aRightHeight;
	}

	// Assign bottom pieces
	for(i=mTexVecWidth*(mTexVecHeight-1); i<(int)mTextures.size(); i++)
	{
		TextureDataPiece &aPiece = mTextures[i];
		aPiece.mWidth = aBottomWidth;
		aPiece.mHeight = aBottomHeight;
	}

	// Assign corner piece
	mTextures.back().mWidth = aCornerWidth;
	mTextures.back().mHeight = aCornerHeight;
/**/

	mMaxTotalU = aWidth/(float)mTexPieceWidth;
	mMaxTotalV = aHeight/(float)mTexPieceHeight;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GLuint TextureData::GetTexture(int x, int y, int &width, int &height, float &u1, float &v1, float &u2, float &v2)
{
	int tx = x/mTexPieceWidth;
	int ty = y/mTexPieceHeight;

	TextureDataPiece &aPiece = mTextures[ty*mTexVecWidth + tx];

	int left = x%mTexPieceWidth;
	int top = y%mTexPieceHeight;
	int right = left+width;
	int bottom = top+height;

	if(right > aPiece.mWidth)
		right = aPiece.mWidth;

	if(bottom > aPiece.mHeight)
		bottom = aPiece.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = left/(float)aPiece.mWidth;
	v1 = top/(float)aPiece.mHeight;
	u2 = right/(float)aPiece.mWidth;
	v2 = bottom/(float)aPiece.mHeight;

	return aPiece.mTexture;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GLuint TextureData::GetTextureF(float x, float y, float &width, float &height, float &u1, float &v1, float &u2, float &v2)
{
  int tx = (int)(x/mTexPieceWidth);
  int ty = (int)(y/mTexPieceHeight);

	TextureDataPiece &aPiece = mTextures[ty*mTexVecWidth + tx];

	float left = x - tx*mTexPieceWidth;
	float top = y - ty*mTexPieceHeight;
	float right = left+width;
	float bottom = top+height;

	if(right > aPiece.mWidth)
		right = aPiece.mWidth;

	if(bottom > aPiece.mHeight)
		bottom = aPiece.mHeight;

	width = right-left;
	height = bottom-top;

	u1 = left/aPiece.mWidth;
	v1 = top/aPiece.mHeight;
	u2 = right/aPiece.mWidth;
	v2 = bottom/aPiece.mHeight;

	return aPiece.mTexture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CreateTextures(MemoryImage *theImage)
{
	theImage->DeleteSWBuffers(); // don't need these buffers for 3d drawing

	theImage->CommitBits();

	// Release texture if image size has changed
	bool createTextures = false;
	if (mWidth!=theImage->mWidth || mHeight!=theImage->mHeight || theImage->mD3DFlags!=mImageFlags)
	{
		ReleaseTextures();
		mImageFlags = theImage->mD3DFlags;
                CreateTextureDimensions(theImage);
		createTextures = true;
	}

	int i,x,y;

	int aHeight = theImage->GetHeight();
	int aWidth = theImage->GetWidth();

	int aFormatSize = 4;
#if 0
	if (aFormat==PixelFormat_Palette8)
		aFormatSize = 1;
	else if (aFormat==PixelFormat_R5G6B5)
		aFormatSize = 2;
	else if (aFormat==PixelFormat_A4R4G4B4)
		aFormatSize = 2;
#endif

	i=0;
	for(y=0; y<aHeight; y+=mTexPieceHeight)
	{
		for(x=0; x<aWidth; x+=mTexPieceWidth, i++)
		{
			TextureDataPiece &aPiece = mTextures[i];
			if (createTextures)
			{

                          //FIXME create texture directly from image without intermediate SDL_Surface

                          SDL_Surface* surface = CreateTextureSurface(aPiece.mWidth, aPiece.mHeight);
                          if (surface == NULL)
                          return;
                          
                          CopyImageToSurface(theImage, surface, x, y, aPiece.mWidth, aPiece.mHeight);

                          aPiece.mTexture = CreateTexture(surface);

				if (aPiece.mTexture==0) // create texture failure
				{
					return;
				}

#if 0
				if (mPalette!=NULL)
					aPiece.mTexture->SetPalette(mPalette);
#endif
					
				mTexMemSize += aPiece.mWidth*aPiece.mHeight*aFormatSize;
			}
		}
	}

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::CheckCreateTextures(MemoryImage *theImage)
{
	if(theImage->mWidth != mWidth || theImage->mHeight != mHeight || theImage->mBitsChangedCount != mBitsChangedCount || theImage->mD3DFlags != mImageFlags)
          CreateTextures(theImage);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TextureData::Blt(float theX, float theY, const Rect& theSrcRect, const Color& theColor)
{
    glEnable(GL_TEXTURE_2D); 	

    int srcLeft = theSrcRect.mX;
	int srcTop = theSrcRect.mY;
	int srcRight = srcLeft + theSrcRect.mWidth;
	int srcBottom = srcTop + theSrcRect.mHeight;
	int srcX, srcY;
	float dstX, dstY;
	int aWidth,aHeight;
	float u1,v1,u2,v2;

	srcY = srcTop;
	dstY = theY;

        SexyRGBA rgba = theColor.ToRGBA();
        glColor4ub(rgba.r, rgba.g, rgba.b, rgba.a);	
	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

	while(srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = theX;
		while(srcX < srcRight)
		{
			aWidth = srcRight-srcX;
			aHeight = srcBottom-srcY;

			GLuint aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

                        float x = dstX;// - 0.5f;
                        float y = dstY;// 0.5f;
                        
                        glBindTexture(GL_TEXTURE_2D, aTexture);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2f(u1, v1);
                        glVertex2f(x,y);
                        glTexCoord2f(u1, v2);
                        glVertex2f(x,y + aHeight);
                        glTexCoord2f(u2, v1);
                        glVertex2f(x + aWidth,y);
                        glTexCoord2f(u2, v2);
                        glVertex2f(x + aWidth,y + aHeight);
                        glEnd();

			srcX += aWidth;
			dstX += aWidth;

		}

		srcY += aHeight;
		dstY += aHeight;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct VertexList
{
	enum { MAX_STACK_VERTS = 100 };
	D3DTLVERTEX mStackVerts[MAX_STACK_VERTS];
	D3DTLVERTEX *mVerts;
	int mSize;
	int mCapacity;

	typedef int size_type;

	VertexList() : mSize(0), mCapacity(MAX_STACK_VERTS), mVerts(mStackVerts) { }
	VertexList(const VertexList &theList) : mSize(theList.mSize), mCapacity(MAX_STACK_VERTS), mVerts(mStackVerts)  
	{ 
		reserve(mSize);
		memcpy(mVerts,theList.mVerts,mSize*sizeof(mVerts[0]));
	}
	
	~VertexList() 
	{ 
		if (mVerts != mStackVerts)
			delete mVerts; 
	}

	void reserve(int theCapacity)
	{
		if (mCapacity < theCapacity)
		{
			mCapacity = theCapacity;
			D3DTLVERTEX *aNewList = new D3DTLVERTEX[theCapacity];
			memcpy(aNewList,mVerts,mSize*sizeof(mVerts[0]));
			if (mVerts != mStackVerts)
				delete mVerts;

			mVerts = aNewList;
		}
	}

	void push_back(const D3DTLVERTEX &theVert) 
	{ 
		if (mSize==mCapacity)
			reserve(mCapacity*2);
			
		mVerts[mSize++] = theVert; 
	}
	
	void operator=(const VertexList &theList) 
	{ 
		reserve(theList.mSize);
		mSize = theList.mSize; 
		memcpy(mVerts,theList.mVerts,mSize*sizeof(mVerts[0]));
	}
	

	D3DTLVERTEX& operator[](int thePos) 
	{ 
		return mVerts[thePos]; 
	}

	int size() { return mSize; }
	void clear() { mSize = 0; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline float GetCoord(const D3DTLVERTEX &theVertex, int theCoord)
{
	switch (theCoord)
	{
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
	aVertex.sx = v1.sx + t*(v2.sx-v1.sx);
	aVertex.sy = v1.sy + t*(v2.sy-v1.sy);
	aVertex.tu = v1.tu + t*(v2.tu-v1.tu);
	aVertex.tv = v1.tv + t*(v2.tv-v1.tv);
	if (v1.color!=v2.color) //FIXME check all members individually
	{

          int r = v1.color.r + (int)(t*(v2.color.r - v1.color.r));
          int g = v1.color.g +(int)( t*(v2.color.g - v1.color.g));
          int b = v1.color.b + (int)(t*(v2.color.b - v1.color.b));
          int a = v1.color.a + (int)(t*(v2.color.a - v1.color.a));

          aVertex.color = Color(r,g,b,a).ToRGBA();
	}
	
	return aVertex;
}
#if 0
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void DisplayError(HRESULT theError, const char *theMsg)
{
	static bool hadError = false;
	if (!hadError)
	{
		std::string aMsg;
		std::string anError = GetDirectXErrorString(theError);
		aMsg = theMsg;
		aMsg += ": ";
		aMsg += anError;

		hadError = true;
		int aResult = MessageBoxA(NULL,aMsg.c_str(),"Error",MB_ABORTRETRYIGNORE);
		if (aResult==IDABORT)
			exit(0);
		else if (aResult==IDRETRY)
			_asm int 3;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool D3DInterface::CheckDXError(HRESULT theError, const char *theMsg)
{
	if(FAILED(theError))
	{
		std::string aMsg;
		std::string anError = GetDirectXErrorString(theError);
		aMsg = theMsg;
		aMsg += ": ";
		aMsg += anError;
		mErrorString = aMsg;
		gSexyAppBase->RegistryWriteString("Test3D\\RuntimeError",aMsg);

	//	DisplayError(theError,theMsg);
		return true;
	}
	else
		return false;
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
D3DInterface::D3DInterface()
{
#if 0
	mHWnd = NULL;
	mDD = NULL;
	mD3D = NULL;
	mD3DDevice = NULL;
#endif
	mWidth = 640;
	mHeight = 480;

	mDDSDrawSurface = NULL;
	mZBuffer = NULL;

	mSceneBegun = false;
	mIsWindowed = true;

	gMinTextureWidth = 64;
	gMinTextureHeight = 64;
	gMaxTextureWidth = 64;
	gMaxTextureHeight = 64;
	gMaxTextureAspectRatio = 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
D3DInterface::~D3DInterface()
{
	Cleanup();
}
#if 0
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::MakeDDPixelFormat(PixelFormat theFormatType, DDPIXELFORMAT* theFormat)
{
	ZeroMemory(theFormat,sizeof(DDPIXELFORMAT));
	theFormat->dwSize = sizeof(DDPIXELFORMAT);

	switch(theFormatType)
	{
		case PixelFormat_A8R8G8B8:
			theFormat->dwFlags = DDPF_ALPHAPIXELS | DDPF_RGB;
			theFormat->dwRGBBitCount = 32;
			theFormat->dwRGBAlphaBitMask	= 0xFF000000;
			theFormat->dwRBitMask			= 0x00FF0000;
			theFormat->dwGBitMask			= 0x0000FF00;
			theFormat->dwBBitMask			= 0x000000FF;
			break;


		case PixelFormat_A4R4G4B4:
			theFormat->dwFlags = DDPF_ALPHAPIXELS | DDPF_RGB;
			theFormat->dwRGBBitCount = 16;
			theFormat->dwRGBAlphaBitMask	= 0xF000;
			theFormat->dwRBitMask			= 0x0F00;
			theFormat->dwGBitMask			= 0x00F0;
			theFormat->dwBBitMask			= 0x000F;
			break;

		case PixelFormat_R5G6B5:
			theFormat->dwFlags = DDPF_RGB;
			theFormat->dwRGBBitCount = 16;
			theFormat->dwRBitMask			= 0xF800;
			theFormat->dwGBitMask			= 0x07E0;
			theFormat->dwBBitMask			= 0x001F;
			break;

		case PixelFormat_Palette8:
			theFormat->dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
			theFormat->dwRGBBitCount = 8;
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PixelFormat D3DInterface::GetDDPixelFormat(LPDDPIXELFORMAT theFormat) 
{
	if (theFormat->dwFlags						==	(DDPF_ALPHAPIXELS | DDPF_RGB)	&& 
		theFormat->dwRGBBitCount				==	32								&& 
		theFormat->dwRGBAlphaBitMask			==	0xFF000000						&&
		theFormat->dwRBitMask					==	0x00FF0000						&&
		theFormat->dwGBitMask					==	0x0000FF00						&&
		theFormat->dwBBitMask					==	0x000000FF)
	{
		return PixelFormat_A8R8G8B8;
	}

	if (theFormat->dwFlags						==	(DDPF_ALPHAPIXELS | DDPF_RGB)	&& 
		theFormat->dwRGBBitCount				==	16								&& 
		theFormat->dwRGBAlphaBitMask			==	0xF000							&&
		theFormat->dwRBitMask					==	0x0F00							&&
		theFormat->dwGBitMask					==	0x00F0							&&
		theFormat->dwBBitMask					==	0x000F)
	{
		return PixelFormat_A4R4G4B4;
	}

	if (theFormat->dwFlags						==	DDPF_RGB						&& 
		theFormat->dwRGBBitCount				==	16								&& 
		theFormat->dwRGBAlphaBitMask			==	0x0000							&&
		theFormat->dwRBitMask					==	0xF800							&&
		theFormat->dwGBitMask					==	0x07E0							&&
		theFormat->dwBBitMask					==	0x001F)
	{
		return PixelFormat_R5G6B5;
	}

	if (theFormat->dwFlags						== (DDPF_RGB | DDPF_PALETTEINDEXED8) &&
		theFormat->dwRGBBitCount				== 8)
	{
		return PixelFormat_Palette8;
	}

	return PixelFormat_Unknown;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HRESULT CALLBACK D3DInterface::PixelFormatsCallback(LPDDPIXELFORMAT theFormat, LPVOID lpContext)
{
	gSupportedPixelFormats |= D3DInterface::GetDDPixelFormat(theFormat);
	
	return D3DENUMRET_OK; 
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::UpdateViewport()
{
#if 0
	HRESULT hr;
	RECT aRect;
	GetClientRect(mHWnd, &aRect);

	POINT aTopLeft = {aRect.left, aRect.top};
	POINT aBotRight = {aRect.right, aRect.bottom};
	::ClientToScreen(mHWnd, &aTopLeft);
	::ClientToScreen(mHWnd, &aBotRight);

	RECT aScreenRect = {aTopLeft.x, aTopLeft.y, aBotRight.x, aBotRight.y};

	D3DVIEWPORT7 &aD3DViewport = mD3DViewport;
	aD3DViewport.dwX = 0;
	aD3DViewport.dwY = 0;
	aD3DViewport.dwWidth = aScreenRect.right - aScreenRect.left;
	aD3DViewport.dwHeight = aScreenRect.bottom - aScreenRect.top;
	aD3DViewport.dvMinZ = 0; //-2048.0f;
	aD3DViewport.dvMaxZ = 1; //2048.0f;

	hr = mD3DDevice->SetViewport(&mD3DViewport);	
#endif

        glViewport(0,0, gSexyAppBase->mWidth, gSexyAppBase->mHeight);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool D3DInterface::InitD3D()
{	

  GLint minimum_width = 1;
  GLint minimum_height = 1;

  while (true) {
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,  minimum_width, minimum_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
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

  GLint try_width = minimum_width;
  GLint try_height = minimum_height;

  while (true) {
    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,  try_width << 1, try_height << 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
    GLint width = 0; 
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); 
    GLint height = 0;
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height); 
    
    if (width != 0)
      try_width <<= 1;
    
    if (height != 0)
      try_height <<= 1;

    if (width == 0 && height == 0)
      break;
  }

  gMinTextureWidth = minimum_width;
  gMinTextureHeight = minimum_height;
  gMaxTextureWidth = try_width;
  gMaxTextureHeight = try_height;
  //FIXME 
  gMaxTextureAspectRatio = 1;

	if (gMaxTextureWidth > MAX_TEXTURE_SIZE)
		gMaxTextureWidth = MAX_TEXTURE_SIZE;
	if (gMaxTextureHeight > MAX_TEXTURE_SIZE)
		gMaxTextureHeight = MAX_TEXTURE_SIZE;

	if (gMaxTextureAspectRatio==0)
		gMaxTextureAspectRatio = 65536;

#if 0
        if (gMinTextureWidth > gMaxTextureWidth)
          gMinTextureWidth = 64;
        if (gMinTextureHeight > gMaxTextureHeight)
          gMinTextureHeight = 64;
#endif

#if 0
	gSupportedPixelFormats = 0;
	mD3DDevice->EnumTextureFormats(PixelFormatsCallback,NULL);

	if (!(aCaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)) // need alpha in palettes
		gSupportedPixelFormats &= ~PixelFormat_Palette8;
#endif

   glEnable (GL_LINE_SMOOTH);
   glEnable(GL_BLEND);
   glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
   glLineWidth (1.0);
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_NORMALIZE);
   glDisable(GL_CULL_FACE);
   glShadeModel (GL_FLAT);
   glReadBuffer(GL_BACK);
   glPixelStorei( GL_PACK_ROW_LENGTH, 0 ) ;
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) ;
   glClear(GL_COLOR_BUFFER_BIT);
   glDisable(GL_TEXTURE_GEN_S);
   glDisable(GL_TEXTURE_GEN_T);

   glClearColor (0.0, 0.0, 0.0, 0.0); 
   UpdateViewport();

   glMatrixMode( GL_PROJECTION ); 
   glLoadIdentity(); 
   gluOrtho2D( 0, gSexyAppBase->mWidth, gSexyAppBase->mHeight, 0 );
   glMatrixMode( GL_MODELVIEW ); 
   glLoadIdentity(); 

   //create texture for mOldCursorArea
   glGenTextures(1, &custom_cursor_texture);
   glBindTexture(GL_TEXTURE_2D, custom_cursor_texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
   unsigned char tmp[64*64*4];
   memset(tmp,0,64*64*4);
   glTexImage2D(GL_TEXTURE_2D,
             0,
             GL_RGBA,
             64, 64,
             0,
             GL_RGBA,
             GL_UNSIGNED_BYTE,
             tmp);
   
   gLinearFilter = false;

#if 0
   // Create ZBuffer
   DDPIXELFORMAT ddpfZBuffer;
   mD3D->EnumZBufferFormats( IID_IDirect3DHALDevice, EnumZBufferCallback, (VOID*)&ddpfZBuffer );

   DDSURFACEDESC2 ddsd;
   ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
   ddsd.dwSize         = sizeof(DDSURFACEDESC2);
   ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
   ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
   ddsd.dwWidth        = mD3DViewport.dwWidth;
   ddsd.dwHeight       = mD3DViewport.dwHeight;
   memcpy( &ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT) );

   mD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET ,0xff000000, 1.0f, 0L);
#endif
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

  //  mIsWindowed = true; //FIXME 
  return InitD3D();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool gD3DInterfacePreDrawError = false;
bool D3DInterface::PreDraw()
{
	if (gSexyAppBase->mPhysMinimized)
		return false;

	if (!mSceneBegun)
	{
#if 0
		HRESULT hr;


		if (!SUCCEEDED(mD3DDevice->SetRenderTarget(mDDSDrawSurface, 0))) // this happens when there's been a mode switch (this caused the nvidia screensaver bluescreen)
		{
			gD3DInterfacePreDrawError = true;
			return false;
		}
		else
#endif
			gD3DInterfacePreDrawError = false;

#if 0
		hr = mD3DDevice->BeginScene();
#endif		

                glDisable(GL_DEPTH);
                glDisable(GL_LIGHTING);
#if 0
		// alphablend states 
		mD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		mD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA); 
		mD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA); 
		mD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING , FALSE);

		// filter states 
		mD3DDevice->SetTextureStageState(0,D3DTSS_MINFILTER, D3DTFG_POINT); 
		mD3DDevice->SetTextureStageState(0,D3DTSS_MAGFILTER, D3DTFG_POINT); 
		mD3DDevice->SetTextureStageState(0,D3DTSS_MIPFILTER, D3DTFG_POINT); 
		mD3DDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE  );
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
		gLinearFilter = false;
	}

	return true;
}
#if 0
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToTexture4444(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, bool rightPad)
{
	if (theImage->mColorTable == NULL)
	{
		DWORD *srcRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;
		char *dstRow = (char*)theDest;

		for(int y=0; y<theHeight; y++)
		{
			DWORD *src = srcRow;
			ushort *dst = (ushort*)dstRow;
			for(int x=0; x<theWidth; x++)
			{
				DWORD aPixel = *src++;
				*dst++ = ((aPixel>>16)&0xF000) | ((aPixel>>12)&0x0F00) | ((aPixel>>8)&0x00F0) | ((aPixel>>4)&0x000F);
			}

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else // palette
	{
		uchar *srcRow = (uchar*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uchar *dstRow = (uchar*)theDest;
		DWORD *palette = theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uchar *src = srcRow;
			ushort *dst = (ushort*)dstRow;
			for(int x=0; x<theWidth; x++)
			{
				DWORD aPixel = palette[*src++];
				*dst++ = ((aPixel>>16)&0xF000) | ((aPixel>>12)&0x0F00) | ((aPixel>>8)&0x00F0) | ((aPixel>>4)&0x000F);
			}

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyTexture4444ToImage(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight)
{		
	char *srcRow = (char*)theDest;
	DWORD *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

	for(int y=0; y<theHeight; y++)
	{
		ushort *src = (ushort*)srcRow;
		DWORD *dst = dstRow;
		
		for(int x=0; x<theWidth; x++)
		{
			ushort aPixel = *src++;			
			*dst++ = 0xFF000000 | ((aPixel & 0xF000) << 16) | ((aPixel & 0x0F00) << 12) | ((aPixel & 0x00F0) << 8) | ((aPixel & 0x000F) << 4);
		}

		dstRow += theImage->GetWidth();
		srcRow += theDestPitch;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToTexture565(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, bool rightPad)
{
	if (theImage->mColorTable == NULL)
	{
		DWORD *srcRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;
		char *dstRow = (char*)theDest;

		for(int y=0; y<theHeight; y++)
		{
			DWORD *src = srcRow;
			ushort *dst = (ushort*)dstRow;
			for(int x=0; x<theWidth; x++)
			{
				DWORD aPixel = *src++;
				*dst++ = ((aPixel>>8)&0xF800) | ((aPixel>>5)&0x07E0) | ((aPixel>>3)&0x001F);
			}

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}
	}
	else // palette
	{
		uchar *srcRow = (uchar*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
		uchar *dstRow = (uchar*)theDest;
		DWORD *palette = theImage->mColorTable;

		for(int y=0; y<theHeight; y++)
		{
			uchar *src = srcRow;
			ushort *dst = (ushort*)dstRow;
			for(int x=0; x<theWidth; x++)
			{
				DWORD aPixel = palette[*src++];
				*dst++ = ((aPixel>>8)&0xF800) | ((aPixel>>5)&0x07E0) | ((aPixel>>3)&0x001F);
			}

			if (rightPad) 
				*dst = *(dst-1);

			srcRow += theImage->GetWidth();
			dstRow += theDestPitch;
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyTexture565ToImage(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight)
{		
	char *srcRow = (char*)theDest;
	DWORD *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

	for(int y=0; y<theHeight; y++)
	{
		ushort *src = (ushort*)srcRow;
		DWORD *dst = dstRow;
		
		for(int x=0; x<theWidth; x++)
		{
			ushort aPixel = *src++;			
			*dst++ = 0xFF000000 | ((aPixel & 0xF800) << 8) | ((aPixel & 0x07E0) << 5) | ((aPixel & 0x001F) << 3);
		}

		dstRow += theImage->GetWidth();
		srcRow += theDestPitch;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyImageToTexturePalette8(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, bool rightPad)
{
	uchar *srcRow = (uchar*)theImage->mColorIndices + offy * theImage->GetWidth() + offx;
	uchar *dstRow = (uchar*)theDest;

	for(int y=0; y<theHeight; y++)
	{
		uchar *src = srcRow;
		uchar *dst = dstRow;
		for(int x=0; x<theWidth; x++)
			*dst++ = *src++;

		if (rightPad) 
			*dst = *(dst-1);

		srcRow += theImage->GetWidth();
		dstRow += theDestPitch;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void CopyTexturePalette8ToImage(void *theDest, DWORD theDestPitch, MemoryImage *theImage, int offx, int offy, int theWidth, int theHeight, LPDIRECTDRAWPALETTE thePalette)
{
	char *srcRow = (char*)theDest;
	DWORD *dstRow = theImage->GetBits() + offy * theImage->GetWidth() + offx;

	PALETTEENTRY aPaletteEntries[256];
	thePalette->GetEntries(0, 0, 256, aPaletteEntries); 

	for(int y=0; y<theHeight; y++)
	{
		uchar *src = (uchar*) srcRow;
		DWORD *dst = dstRow;
		
		for(int x=0; x<theWidth; x++)
		{
			DWORD aPixel = *((DWORD*)(aPaletteEntries+*src++));
			*dst++ = (aPixel&0xFF00FF00) | ((aPixel>>16)&0xFF) | ((aPixel<<16)&0xFF0000);
		}

		dstRow += theImage->GetWidth();
		srcRow += theDestPitch;
	}
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class Pred>
struct PointClipper
{
	Pred mPred;

	void ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList &out);
	void ClipPoints(int n, float clipVal, VertexList &in, VertexList &out);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class Pred>
void PointClipper<Pred>::ClipPoint(int n, float clipVal, const D3DTLVERTEX &v1, const D3DTLVERTEX &v2, VertexList &out)
{
  if (!mPred(GetCoord(v1,n), clipVal))
	{
		if (!mPred(GetCoord(v2,n), clipVal)) // both inside
			out.push_back(v2);
		else // inside -> outside
		{
			float t = (clipVal - GetCoord(v1,n))/(GetCoord(v2,n)-GetCoord(v1,n));
			out.push_back(Interpolate(v1,v2,t));
		}
	}
	else
	{
		if (!mPred(GetCoord(v2,n), clipVal)) // outside -> inside
		{
			float t = (clipVal - GetCoord(v1, n))/(GetCoord(v2,n)-GetCoord(v1,n));
			out.push_back(Interpolate(v1,v2,t));
			out.push_back(v2);
		}
//			else // outside -> outside
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template<class Pred>
void PointClipper<Pred>::ClipPoints(int n, float clipVal, VertexList &in, VertexList &out)
{
	if(in.size()<2)
		return;

	ClipPoint(n,clipVal,in[in.size()-1],in[0],out);
	for(VertexList::size_type i=0; i<in.size()-1; i++)
		ClipPoint(n,clipVal,in[i],in[i+1],out);
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

	aLessClipper.ClipPoints(0,left,*in,*out); std::swap(in,out); out->clear();
	aLessClipper.ClipPoints(1,top,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(0,right,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(1,bottom,*in,*out); 

	VertexList &aList = *out;

        if (aList.size() >= 3) {
          glBegin(GL_TRIANGLE_FAN);
          for (int i = 0; i < aList.size(); ++i) {
            glColor4ub(aList[i].color.r, aList[i].color.g, aList[i].color.b, aList[i].color.a);
            glTexCoord2f(aList[i].tu, aList[i].tv);
            glVertex2f(aList[i].sx, aList[i].sy);
          }
          glEnd();
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

	aLessClipper.ClipPoints(3,left,*in,*out); std::swap(in,out); out->clear();
	aLessClipper.ClipPoints(4,top,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(3,right,*in,*out); std::swap(in,out); out->clear();
	aGreaterClipper.ClipPoints(4,bottom,*in,*out); 
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
	float u1,v1,u2,v2;
	float startx = 0, starty = 0;
	float pixelcorrect = 0.0f;//0.5f;

	if (center)
	{
		startx = -theSrcRect.mWidth/2.0f;
		starty = -theSrcRect.mHeight/2.0f;
		pixelcorrect = 0.0f;
	}			

	srcY = srcTop;
	dstY = starty;

	SexyRGBA rgba = theColor.ToRGBA();		

	if ((srcLeft >= srcRight) || (srcTop >= srcBottom))
		return;

        glEnable(GL_TEXTURE_2D); 
        glColor4ub(rgba.r, rgba.g, rgba.b, rgba.a);

	while(srcY < srcBottom)
	{
		srcX = srcLeft;
		dstX = startx;
		while(srcX < srcRight)
		{
			aWidth = srcRight-srcX;
			aHeight = srcBottom-srcY;
			GLuint aTexture = GetTexture(srcX, srcY, aWidth, aHeight, u1, v1, u2, v2);

			float x = dstX;// - pixelcorrect; // - 0.5f; //FIXME correct??
			float y = dstY;// - pixelcorrect; // - 0.5f;

			SexyVector2 p[4] = { SexyVector2(x, y), SexyVector2(x,y+aHeight), SexyVector2(x+aWidth, y) , SexyVector2(x+aWidth, y+aHeight) };
			SexyVector2 tp[4];

			int i;
			for (i=0; i<4; i++)
			{
				tp[i] = theTrans*p[i];
				tp[i].x -= pixelcorrect - theX;
				tp[i].y -= pixelcorrect - theY;
			}

			bool clipped = false;
			if (theClipRect != NULL)
			{
				int left = theClipRect->mX;
				int right = left + theClipRect->mWidth;
				int top = theClipRect->mY;
				int bottom = top + theClipRect->mHeight;
				for (i=0; i<4; i++)
				{
					if (tp[i].x<left || tp[i].x>=right || tp[i].y<top || tp[i].y>=bottom)
					{
						clipped = true;
						break;
					}
				}
			}

                        glBindTexture(GL_TEXTURE_2D, aTexture);

                        if (!clipped) {
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2f(u1, v1);
                        glVertex2f(tp[0].x,tp[0].y);
                        glTexCoord2f(u1, v2);
                        glVertex2f(tp[1].x,tp[1].y);
                        glTexCoord2f(u2, v1);
                        glVertex2f(tp[2].x,tp[2].y);
                        glTexCoord2f(u2, v2);
                        glVertex2f(tp[3].x,tp[3].y); 
                        glEnd();
                        }
                        else {
				VertexList aList;

                                D3DTLVERTEX vertex0 = {(GLfloat)u1,(GLfloat)v1, rgba, (GLfloat)tp[0].x, (GLfloat)tp[0].y};
                                D3DTLVERTEX vertex1 = {(GLfloat)u1,(GLfloat)v2, rgba, (GLfloat)tp[1].x, (GLfloat)tp[1].y};
                                D3DTLVERTEX vertex2 = {(GLfloat)u2,(GLfloat)v1, rgba, (GLfloat)tp[2].x, (GLfloat)tp[2].y};
                                D3DTLVERTEX vertex3 = {(GLfloat)u2,(GLfloat)v2, rgba, (GLfloat)tp[3].x, (GLfloat)tp[3].y};
                                
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
#define GetColorFromTriVertex(theVertex, theColor) (theVertex.color?theVertex.color:theColor)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TextureData::BltTriangles(const TriVertex theVertices[][3], int theNumTriangles, Uint32 theColor, float tx, float ty)
{      
  glEnable(GL_TEXTURE_2D); //FIXME only set this at start of drawing all 

  if ((mMaxTotalU <= 1.0) && (mMaxTotalV <= 1.0))
	{
          glBindTexture(GL_TEXTURE_2D, mTextures[0].mTexture);

		D3DTLVERTEX aVertexCache[300];
		int aVertexCacheNum = 0;
                          glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                          glEnableClientState(GL_COLOR_ARRAY);
                          glEnableClientState(GL_VERTEX_ARRAY);
  
                          glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte), aVertexCache);
                          glColorPointer(4, GL_UNSIGNED_BYTE, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte), ((GLubyte*)aVertexCache) + 2 * sizeof(GLfloat));
                        glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat) + 4 * sizeof(GLubyte), ((GLubyte*)aVertexCache) + 2 * sizeof(GLfloat) + 4 * sizeof(GLubyte));

		for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
		{
                  Color col;
			TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];
			D3DTLVERTEX* aD3DVertex = &aVertexCache[aVertexCacheNum];
                        aVertexCacheNum += 3;

			aD3DVertex[0].sx = aTriVerts[0].x + tx;
			aD3DVertex[0].sy = aTriVerts[0].y + ty;
			aD3DVertex[0].sz = 0;
                        //		aD3DVertex[0].rhw = 1;
                        col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aD3DVertex[0].color = col.ToRGBA();
			//aD3DVertex[0].specular = 0;
			aD3DVertex[0].tu = aTriVerts[0].u * mMaxTotalU;
			aD3DVertex[0].tv = aTriVerts[0].v * mMaxTotalV;

			aD3DVertex[1].sx = aTriVerts[1].x + tx;
			aD3DVertex[1].sy = aTriVerts[1].y + ty;
			aD3DVertex[1].sz = 0;
			//aD3DVertex[1].rhw = 1;
                        col = GetColorFromTriVertex(aTriVerts[0],theColor); 
			aD3DVertex[1].color = col.ToRGBA();
			//aD3DVertex[1].specular = 0;
			aD3DVertex[1].tu = aTriVerts[1].u * mMaxTotalU;
			aD3DVertex[1].tv = aTriVerts[1].v * mMaxTotalV;

			aD3DVertex[2].sx = aTriVerts[2].x + tx;
			aD3DVertex[2].sy = aTriVerts[2].y + ty;
			aD3DVertex[2].sz = 0;
			//aD3DVertex[2].rhw = 1;
                        col = GetColorFromTriVertex(aTriVerts[0],theColor);
			aD3DVertex[2].color = col.ToRGBA();
			//aD3DVertex[2].specular = 0;
			aD3DVertex[2].tu = aTriVerts[2].u * mMaxTotalU;
			aD3DVertex[2].tv = aTriVerts[2].v * mMaxTotalV;
			
			if ((aVertexCacheNum == 300) || (aTriangleNum == theNumTriangles - 1))
                          {

                        
                            glDrawArrays(GL_TRIANGLES, 0, 300);

                            if (aTriangleNum == theNumTriangles - 1) {
                              glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                              glDisableClientState(GL_COLOR_ARRAY);
                              glDisableClientState(GL_VERTEX_ARRAY);
                            }

                        
                            aVertexCacheNum = 0; 
                          }
                }
        }
  else
    {
      for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
        {
          TriVertex* aTriVerts = (TriVertex*) theVertices[aTriangleNum];

          D3DTLVERTEX aVertex[3]; 
                        Color col = GetColorFromTriVertex(aTriVerts[0],theColor);

                        D3DTLVERTEX vertex1 = {(GLfloat)(aTriVerts[0].u * mMaxTotalU), (GLfloat)(aTriVerts[0].v * mMaxTotalV),
                                                                                   col.ToRGBA(),
                                                                                   aTriVerts[0].x + tx, aTriVerts[0].u + ty, 0.0f};
                        
                        col = GetColorFromTriVertex(aTriVerts[1],theColor);
                        
                        D3DTLVERTEX vertex2 = {(GLfloat)(aTriVerts[1].u * mMaxTotalU), (GLfloat)(aTriVerts[1].v * mMaxTotalV),
                                               col.ToRGBA(),
                                               aTriVerts[1].x + tx, aTriVerts[1].u + ty, 0.0f};
                        col = GetColorFromTriVertex(aTriVerts[2],theColor);

                        D3DTLVERTEX vertex3 = {(GLfloat)(aTriVerts[2].u * mMaxTotalU), (GLfloat)(aTriVerts[2].v * mMaxTotalV),
                                               col.ToRGBA(),
                                               aTriVerts[2].x + tx, aTriVerts[2].u + ty, 0.0f};

                        aVertex[0] = vertex1;
                        aVertex[1]  = vertex2;
                        aVertex[2] = vertex3;

			float aMinU = mMaxTotalU, aMinV = mMaxTotalV;
			float aMaxU = 0, aMaxV = 0;

			int i,j,k;
			for (i=0; i<3; i++)
			{
				if(aVertex[i].tu < aMinU)
					aMinU = aVertex[i].tu;

				if(aVertex[i].tv < aMinV)
					aMinV = aVertex[i].tv;

				if(aVertex[i].tu > aMaxU)
					aMaxU = aVertex[i].tu;

				if(aVertex[i].tv > aMaxV)
					aMaxV = aVertex[i].tv;
			}
			
			VertexList aMasterList;
			aMasterList.push_back(aVertex[0]);
			aMasterList.push_back(aVertex[1]);
			aMasterList.push_back(aVertex[2]);

			VertexList aList;

			int aLeft = (int)floorf(aMinU);
			int aTop = (int)floorf(aMinV);
			int aRight = (int)ceilf(aMaxU);
			int aBottom = (int)ceilf(aMaxV);
			if (aLeft < 0)
				aLeft = 0;
			if (aTop < 0)
				aTop = 0;
			if (aRight > mTexVecWidth)
				aRight = mTexVecWidth;
			if (aBottom > mTexVecHeight)
				aBottom = mTexVecHeight;

			TextureDataPiece &aStandardPiece = mTextures[0];
			for (i=aTop; i<aBottom; i++)
			{
				for (j=aLeft; j<aRight; j++)
				{
					TextureDataPiece &aPiece = mTextures[i*mTexVecWidth + j];


					VertexList aList = aMasterList;
					for(k=0; k<3; k++)
					{
						aList[k].tu -= j;
						aList[k].tv -= i;
						if (i==mTexVecHeight-1)
							aList[k].tv *= (float)aStandardPiece.mHeight / aPiece.mHeight;
						if (j==mTexVecWidth-1)
							aList[k].tu *= (float)aStandardPiece.mWidth / aPiece.mWidth;
					}
			
                                        DoPolyTextureClip(aList);
					if (aList.size() >= 3)
					{
                                          glBindTexture(GL_TEXTURE_2D, aPiece.mTexture);
                                          glBegin(GL_TRIANGLE_FAN);
                                          for (int i = 0; i < aList.size(); ++i) {//FIXME optimize
                                            glTexCoord2f(aList[i].tu, aList[i].tv);
                                            glColor4ub(aList[i].color.r, aList[i].color.g, aList[i].color.b, aList[i].color.a);
                                            glVertex3f(aList[i].sx, aList[i].sy, aList[i].sz);
                                          }
                                          glEnd();
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

	if(theImage->mD3DData==NULL)
	{
		theImage->mD3DData = new TextureData();
		
		// The actual purging was deferred
		wantPurge = theImage->mPurgeBits;

#if 0
		AutoCrit aCrit(gSexyAppBase->mDDInterface->mCritSect); // Make images thread safe
#endif
		mImageSet.insert(theImage);
	}

	TextureData *aData = (TextureData*)theImage->mD3DData;
	aData->CheckCreateTextures(theImage);
	
	if (wantPurge)
		theImage->PurgeBits();

        //FIXME
	return true;//aData->mPixelFormat != PixelFormat_Unknown;
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
	
	for (int aPieceRow = 0; aPieceRow < aData->mTexVecHeight; aPieceRow++)
	{
		for (int aPieceCol = 0; aPieceCol < aData->mTexVecWidth; aPieceCol++)
		{
			TextureDataPiece* aPiece = &aData->mTextures[aPieceRow*aData->mTexVecWidth + aPieceCol];

			int offx = aPieceCol*aData->mTexPieceWidth;
			int offy = aPieceRow*aData->mTexPieceHeight;
			int aWidth = std::min(theImage->mWidth-offx, aPiece->mWidth);
			int aHeight = std::min(theImage->mHeight-offy, aPiece->mHeight);

			CopySurface8888ToImage(aPiece->mTexture, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight); break;
			case PixelFormat_A4R4G4B4:	CopyTexture4444ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight); break;
			case PixelFormat_R5G6B5: CopyTexture565ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight); break;
			case PixelFormat_Palette8:	CopyTexturePalette8ToImage(aDesc.lpSurface, aDesc.lPitch, theImage, offx, offy, aWidth, aHeight, aData->mPalette); break;
			}


		}
	}
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::SetCurTexture(MemoryImage *theImage)
{
	if (theImage==NULL)
          {
            glBindTexture(GL_TEXTURE_2D, 0);        
            return;
          }

	if (!CreateImageTexture(theImage))
		return;

	TextureData *aData = (TextureData*)theImage->mD3DData;

        glBindTexture(GL_TEXTURE_2D, aData->mTextures[0].mTexture);        
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::PushTransform(const SexyMatrix3 &theTransform, bool concatenate)
{
	if (mTransformStack.empty() || !concatenate)
		mTransformStack.push_back(theTransform);
	else
	{
		SexyMatrix3 &aTrans = mTransformStack.back();
		mTransformStack.push_back(theTransform*aTrans);
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
	if (theImage->mD3DData != NULL)
	{
		delete (TextureData*)theImage->mD3DData;
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
	for(anItr = mImageSet.begin(); anItr != mImageSet.end(); ++anItr)
	{
		MemoryImage *anImage = *anItr;
		delete (TextureData*)anImage->mD3DData;
		anImage->mD3DData = NULL;
	}

	mImageSet.clear();

#if 0
	if (mD3DDevice != NULL)
	{
		mD3DDevice->Release();
		mD3DDevice = NULL;
	}

	if (mD3D != NULL)
	{
		mD3D->Release();
		mD3D = NULL;
	}
#endif
	if (mDDSDrawSurface != NULL)
	{
          SDL_FreeSurface(mDDSDrawSurface);
		mDDSDrawSurface = NULL;
	}

	if (mZBuffer != NULL)
	{
          SDL_FreeSurface(mZBuffer);
		mZBuffer = NULL;
	}

        glDeleteTextures(1, &custom_cursor_texture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::SetupDrawMode(int theDrawMode, const Color &theColor, Image *theImage)
{
  if (theDrawMode == Graphics::DRAWMODE_NORMAL)
    {
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   }
  else // Additive
    {
      glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA); 
    }			
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::Blt(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	if (!mTransformStack.empty())
	{
		BltClipF(theImage,theX,theY,theSrcRect,NULL,theColor,theDrawMode);
		return;
	}

	if (!PreDraw())
		return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetupDrawMode(theDrawMode, theColor, theImage);	

	TextureData *aData = (TextureData*)aSrcMemoryImage->mD3DData;

        SetLinearFilter(linearFilter);

        aData->Blt(theX,theY,theSrcRect,theColor);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::BltMirror(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Color& theColor, int theDrawMode, bool linearFilter)
{
	SexyTransform2D aTransform;		

	aTransform.Translate(-theSrcRect.mWidth,0);
	aTransform.Scale(-1, 1);
	aTransform.Translate(theX, theY);

	BltTransformed(theImage,NULL,theColor,theDrawMode,theSrcRect,aTransform,linearFilter);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::BltClipF(Image* theImage, float theX, float theY, const Rect& theSrcRect, const Rect *theClipRect, const Color& theColor, int theDrawMode)
{
	SexyTransform2D aTransform;
	aTransform.Translate(theX, theY);

        BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::StretchBlt(Image* theImage,  const Rect& theDestRect, const Rect& theSrcRect, const Rect* theClipRect, const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
	float xScale = (float)theDestRect.mWidth / theSrcRect.mWidth;
	float yScale = (float)theDestRect.mHeight / theSrcRect.mHeight;

	SexyTransform2D aTransform;
	if (mirror)
	{
		aTransform.Translate(-theSrcRect.mWidth,0);
		aTransform.Scale(-xScale, yScale);
	}
	else
		aTransform.Scale(xScale, yScale);

	aTransform.Translate(theDestRect.mX, theDestRect.mY);
	BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,!fastStretch);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::BltRotated(Image* theImage, float theX, float theY, const Rect* theClipRect, const Color& theColor, int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY, const Rect &theSrcRect)
{
	SexyTransform2D aTransform;

	aTransform.Translate(-theRotCenterX, -theRotCenterY);
	aTransform.RotateRad(theRot);
	aTransform.Translate(theX+theRotCenterX,theY+theRotCenterY);

	BltTransformed(theImage,theClipRect,theColor,theDrawMode,theSrcRect,aTransform,true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::BltTransformed(Image* theImage, const Rect* theClipRect, const Color& theColor, int theDrawMode, const Rect &theSrcRect, const SexyMatrix3 &theTransform, bool linearFilter, float theX, float theY, bool center)
{
	if (!PreDraw())
		return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*) theImage;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetupDrawMode(theDrawMode, theColor, theImage);	

	TextureData *aData = (TextureData*)aSrcMemoryImage->mD3DData;

	if (!mTransformStack.empty())
	{

          SetLinearFilter(true); // force linear filtering in the case of a global transform

		if (theX!=0 || theY!=0)
		{
			SexyTransform2D aTransform;
			if (center)
				aTransform.Translate(-theSrcRect.mWidth/2.0f,-theSrcRect.mHeight/2.0f);

			aTransform = theTransform * aTransform;
			aTransform.Translate(theX,theY);
			aTransform = mTransformStack.back() * aTransform;

			aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect);
		}
		else
		{
			SexyTransform2D aTransform = mTransformStack.back()*theTransform;
			aData->BltTransformed(aTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
		}
	}
	else
	{

		SetLinearFilter(linearFilter);

		aData->BltTransformed(theTransform, theSrcRect, theColor, theClipRect, theX, theY, center);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color& theColor, int theDrawMode)
{
	if (!PreDraw())
		return;

        glDisable(GL_TEXTURE_2D);

	SetupDrawMode(theDrawMode, theColor, NULL);

	float x1, y1, x2, y2;
	SexyRGBA aColor = theColor.ToRGBA();			

	if (!mTransformStack.empty())
	{
		SexyVector2 p1(theStartX,theStartY);
		SexyVector2 p2(theEndX,theEndY);
		p1 = mTransformStack.back()*p1;
		p2 = mTransformStack.back()*p2;

		x1 = p1.x;
		y1 = p1.y;
		x2 = p2.x;
		y2 = p2.y;
	}
	else
	{
		x1 = theStartX;
		y1 = theStartY;
		x2 = theEndX;
		y2 = theEndY;
	}
        
        glColor4ub(aColor.r, aColor.g, aColor.b, aColor.a);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        //        glVertex2f(x2+0.5f, y2+0.5f);
        glEnd();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::FillRect(const Rect& theRect, const Color& theColor, int theDrawMode)
{
	if (!PreDraw())
		return;

        glDisable(GL_TEXTURE_2D);

	SetupDrawMode(theDrawMode, theColor, NULL);

	SexyRGBA aColor = theColor.ToRGBA();			

	float x = theRect.mX;// - 0.5f;
	float y = theRect.mY;// - 0.5f;
	float aWidth = theRect.mWidth;
	float aHeight = theRect.mHeight;

	D3DTLVERTEX aVertex[4] = 
	{
          { 0,0,aColor,x,				y,0},
          { 0,0,aColor,x,				y + aHeight,0},
          { 0,0,aColor,x + aWidth,				y,0},
          { 0,0,aColor,x + aWidth,				y + aHeight,0}
	};

	if (!mTransformStack.empty())
	{
		SexyVector2 p[4] = { SexyVector2(x, y), SexyVector2(x,y+aHeight), SexyVector2(x+aWidth, y) , SexyVector2(x+aWidth, y+aHeight) };

		int i;
		for (i=0; i<4; i++)
		{
			p[i] = mTransformStack.back()*p[i];
                        //			p[i].x -= 0.5f;
			//p[i].y -= 0.5f;
			aVertex[i].sx = p[i].x;
			aVertex[i].sy = p[i].y;
		}
	}

        glColor4ub(aColor.r, aColor.g, aColor.b, aColor.a);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex2f(aVertex[0].sx, aVertex[0].sy);
        glVertex2f(aVertex[1].sx, aVertex[1].sy);
        glVertex2f(aVertex[2].sx, aVertex[2].sy);
        glVertex2f(aVertex[3].sx, aVertex[3].sy);
        glEnd();
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

	D3DTLVERTEX aVertex[3] = 
	{
          { 0,0,aRGBA1, p1.x,			p1.y,			0},
          { 0,0,aRGBA2, p2.x,			p2.y,			0},
          { 0,0,aRGBA3, p3.x,			p3.y,			0}
	};


        glDisable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLE_STRIP);
        glColor4ub(aRGBA1.r, aRGBA1.g, aRGBA1.b, aRGBA1.a);
        glVertex2f(aVertex[0].sx, aVertex[0].sy);
        glColor4ub(aRGBA2.r, aRGBA2.g, aRGBA2.b, aRGBA2.a);
        glVertex2f(aVertex[1].sx, aVertex[1].sy);
        glColor4ub(aRGBA3.r, aRGBA3.g, aRGBA3.b, aRGBA3.a);
        glVertex2f(aVertex[2].sx, aVertex[2].sy);
        glEnd();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor, int theDrawMode, int tx, int ty)
{
	if (theNumVertices<3)
		return;

	if (!PreDraw())
		return;

	SetupDrawMode(theDrawMode, theColor, NULL);
	SexyRGBA aColor = theColor.ToRGBA();			

	VertexList aList;
	for (int i=0; i<theNumVertices; i++)
	{
          D3DTLVERTEX vert = 	{ 0,0, aColor, theVertices[i].mX + tx, theVertices[i].mY + ty,	0};
		if (!mTransformStack.empty())
		{
			SexyVector2 v(vert.sx,vert.sy);
			v = mTransformStack.back()*v;
			vert.sx = v.x;
			vert.sy = v.y;
		}

		aList.push_back(vert);
	}

	if (theClipRect != NULL)
		DrawPolyClipped(theClipRect,aList);
	else {
          glDisable(GL_TEXTURE_2D);
          glColor4ub(aColor.r, aColor.g, aColor.b, aColor.a);
          glBegin(GL_TRIANGLE_FAN);
          for (int i = 0; i < aList.size(); ++i) { 
            glVertex2f(aList[i].sx, aList[i].sy);
          }
          glEnd();
        }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor, int theDrawMode, Image *theTexture, bool blend)
{
	TriVertex aVertices[1][3] = {{p1, p2, p3}};
	DrawTrianglesTex(aVertices,1,theColor,theDrawMode,theTexture,blend);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	if (!PreDraw())
		return;

	MemoryImage* aSrcMemoryImage = (MemoryImage*)theTexture;

	if (!CreateImageTexture(aSrcMemoryImage))
		return;

	SetupDrawMode(theDrawMode, theColor, theTexture);
	
	TextureData *aData = (TextureData*)aSrcMemoryImage->mD3DData;

	SetLinearFilter(blend);	

	aData->BltTriangles(theVertices, theNumTriangles, (Uint32)theColor.ToInt(), tx, ty);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor, int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	TriVertex aList[100][3];
	int aTriNum = 0;
	while (aTriNum < theNumTriangles)
	{
          int aMaxTriangles = std::min(100,theNumTriangles - aTriNum);
		for (int i=0; i<aMaxTriangles; i++)
		{
			aList[i][0] = theVertices[aTriNum];
			aList[i][1] = theVertices[aTriNum+1];
			aList[i][2] = theVertices[aTriNum+2];
			aTriNum++;
		}
		DrawTrianglesTex(aList,aMaxTriangles,theColor,theDrawMode,theTexture, tx, ty, blend);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void D3DInterface::Flush()
{
	if (mSceneBegun)
	{
#if 0
		mD3DDevice->EndScene();
#endif
		mSceneBegun = false;
		mErrorString.erase();
	}
}


