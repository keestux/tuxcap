#define XMD_H

#include <SDL_image.h>
#include "ImageLib.h"
#include <math.h>
#include "Common.h"
#include "SexyAppBase.h"

using namespace ImageLib;

ImageLib::Image::Image()
{
	mWidth = 0;
	mHeight = 0;
	mBits = NULL;
}


ImageLib::Image::Image(int width, int height) {
  mWidth = width;
  mHeight = height;
  mBits = new uint32_t[mWidth * mHeight];
}


ImageLib::Image::~Image()
{
	delete[] mBits;
}

int	ImageLib::Image::GetWidth()
{
	return mWidth;
}

int	ImageLib::Image::GetHeight()
{
	return mHeight;
}

uint32_t* ImageLib::Image::GetBits()
{
	return mBits;
}

int ImageLib::gAlphaComposeColor = 0xFFFFFF;
bool ImageLib::gAutoLoadAlpha = true;
bool ImageLib::gIgnoreJPEG2000Alpha = true;

ImageLib::Image* ImageLib::GetImage(std::string theFilename, bool lookForAlphaImage)
{

	if (!gAutoLoadAlpha)
		lookForAlphaImage = false;

        theFilename = Sexy::ReplaceBackSlashes(theFilename);

	int aLastDotPos = theFilename.rfind('.');
	int aLastSlashPos = (int)theFilename.rfind('/');

	std::string anExt;
	std::string aFilename;

	if (aLastDotPos > aLastSlashPos)
	{
		anExt = theFilename.substr(aLastDotPos, theFilename.length() - aLastDotPos);
		aFilename = theFilename.substr(0, aLastDotPos);
	}
	else {
          aFilename = theFilename;
        
        }

        ImageLib::Image* anImage = NULL;

        SDL_Surface* mImage = NULL;
        bool ok = false;

        if (anExt.length() == 0) 
          {
            std::list<std::string> coderList;

            coderList.push_back("jpg");
            coderList.push_back("png");
            coderList.push_back("gif");

            std::list<std::string>::const_iterator entry = coderList.begin();
            while( entry != coderList.end() ) 
              {
		  if (Sexy::gSexyAppBase->FileExists(theFilename + "." + *entry))  {
		    mImage = IMG_Load( (theFilename + "." + *entry).c_str());
		  }
		  else if (Sexy::gSexyAppBase->FileExists(theFilename + "." + Sexy::Upper(*entry))) {
		    mImage = IMG_Load( (theFilename + "." + Sexy::Upper(*entry)).c_str());
		  }

		if (mImage) {
		  ok  = true;
		  break;
		}
		
                ++entry;
              }
          }
        else
          {
	      if (Sexy::gSexyAppBase->FileExists(theFilename)) {
		mImage = IMG_Load ( theFilename.c_str() );	  
	      }
	      else if (Sexy::gSexyAppBase->FileExists(aFilename + Sexy::Lower(anExt))) {
		mImage = IMG_Load (  (aFilename + Sexy::Lower(anExt)).c_str() );
	      }
	      else if (Sexy::gSexyAppBase->FileExists(aFilename + Sexy::Upper(anExt))) {
		mImage = IMG_Load (  (aFilename + Sexy::Upper(anExt)).c_str()  );
	      }

	    if (!mImage)
	      return NULL;
	    ok  = true;
	  }

	// Check for alpha images
	Image* anAlphaImage = NULL;
	if(lookForAlphaImage)
	{
          // Check _ImageName
          anAlphaImage = GetImage(aFilename + "_", false);

	  if(anAlphaImage==NULL) {
	    anAlphaImage = GetImage(aFilename.substr(0, aLastSlashPos+1) + "_" +
				    aFilename.substr(aLastSlashPos+1, aFilename.length() - aLastSlashPos - 1), false);
          }
	}

	// Compose alpha channel with image
	if (anAlphaImage != NULL) 
	{

          if (ok && anImage == NULL) {
            //TODO put this in a function
            anImage = new ImageLib::Image(mImage->w, mImage->h);

	    Uint32 temp,pixel;
	    Uint8 red,green,blue,alpha;
	    SDL_Color color;
	    SDL_PixelFormat* fmt = mImage->format;

	    if (fmt->BytesPerPixel == 1) {

	      for(int i = 0; i < mImage->h; ++i) {
		for (int j = 0; j < mImage->w; ++j) {
		  Uint8 index = *(((Uint8*)mImage->pixels) + i*mImage->pitch + j);
		  color=fmt->palette->colors[index];

		  if (mImage->flags & SDL_SRCCOLORKEY) {
		    pixel = SDL_MapRGB(fmt, color.r, color.g, color.b);
		    if (pixel == fmt->colorkey)
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 0;		 
		    else { 
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = color.r;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = color.g;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = color.b;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
		    }
		  }   
		  else 
		    {
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = color.r;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = color.g;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = color.b;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
		  }
		}
	      }

	    }
	    else if (fmt->BytesPerPixel == 3) {

	      for(int i = 0; i < mImage->h; ++i) {
		for (int j = 0; j < mImage->w; ++j) {

		Uint8* p = (((Uint8*)mImage->pixels) + i*mImage->pitch + j * 3);
#if __BIG_ENDIAN__
		pixel =  p[0] << 16 | p[1] << 8 | p[2]; 
#else
		pixel =  p[0] | p[1] << 8 | p[2] << 16; 
#endif
		  /* Get Red component */ 
		  temp=pixel&fmt->Rmask; /* Isolate red component */ 
		  temp=temp>>fmt->Rshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Rloss; /* Expand to a full 8-bit number */ 
		  red=(Uint8)temp; 
		  /* Get Green component */ 
		  temp=pixel&fmt->Gmask; /* Isolate green component */ 
		  temp=temp>>fmt->Gshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Gloss; /* Expand to a full 8-bit number */ 
		  green=(Uint8)temp; 
		  /* Get Blue component */ 
		  temp=pixel&fmt->Bmask; /* Isolate blue component */ 
		  temp=temp>>fmt->Bshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Bloss; /* Expand to a full 8-bit number */ 
		  blue=(Uint8)temp; 

		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = red;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = green;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = blue;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
		  }
		}
	    }
	    else {
	      assert(fmt->BytesPerPixel == 4);

	      for(int i = 0; i < mImage->h; ++i) {
		for (int j = 0; j < mImage->w; ++j) {

		  pixel = *(((Uint32*)mImage->pixels) + i*(mImage->pitch/sizeof(Uint32)) + j);

		  /* Get Red component */ 
		  temp=pixel&fmt->Rmask; /* Isolate red component */ 
		  temp=temp>>fmt->Rshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Rloss; /* Expand to a full 8-bit number */ 
		  red=(Uint8)temp; 

		  /* Get Green component */ 
		  temp=pixel&fmt->Gmask; /* Isolate green component */ 
		  temp=temp>>fmt->Gshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Gloss; /* Expand to a full 8-bit number */ 
		  green=(Uint8)temp; 
		  /* Get Blue component */ 
		  temp=pixel&fmt->Bmask; /* Isolate blue component */ 
		  temp=temp>>fmt->Bshift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Bloss; /* Expand to a full 8-bit number */ 
		  blue=(Uint8)temp; 
		  /* Get Alpha component */ 
		  temp=pixel&fmt->Amask; /* Isolate alpha component */ 
		  temp=temp>>fmt->Ashift;/* Shift it down to 8-bit */ 
		  temp=temp<<fmt->Aloss; /* Expand to a full 8-bit number */ 
		  alpha=(Uint8)temp; 

		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = red;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = green;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = blue;          
		  *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = alpha;
		}
	      }
	    }
	  }
	  if (anImage != NULL)
	    {
		if ((anImage->mWidth == anAlphaImage->mWidth) &&
		    (anImage->mHeight == anAlphaImage->mHeight))
		  {
		    uint32_t* aBits1 = anImage->mBits;
		    uint32_t* aBits2 = anAlphaImage->mBits;
		    int aSize = anImage->mWidth*anImage->mHeight;

		    for (int i = 0; i < aSize; i++)
		      {
#if __BIG_ENDIAN__
			*aBits1 = (*aBits1 & 0xFFFFFF00) | ((*aBits2 >> 24) & 0xFF);
#else
			*aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
#endif
			++aBits1;
			++aBits2;
		      }
		  }

		delete anAlphaImage;
	      }
	    else if (gAlphaComposeColor==0xFFFFFF)
	      {

		anImage = anAlphaImage;
		uint32_t* aBits1 = anImage->mBits;

		int aSize = anImage->mWidth*anImage->mHeight;
		for (int i = 0; i < aSize; i++)
		  {
#if __BIG_ENDIAN__
		    *aBits1 = (0xFFFFFF00) | ((*aBits1 >> 24) & 0xFF);
#else
		    *aBits1 = (0x00FFFFFF) | ((*aBits1 & 0xFF) << 24);
#endif
		    ++aBits1;
		  }
	      }

	    else
	      {

		const int aColor = gAlphaComposeColor;
		anImage = anAlphaImage;
		uint32_t* aBits1 = anImage->mBits;

		int aSize = anImage->mWidth*anImage->mHeight;
		for (int i = 0; i < aSize; i++)
		  {
#if __BIG_ENDIAN__
		    *aBits1 = aColor | ((*aBits1 >> 24) & 0xFF);
#else
		    *aBits1 = aColor | ((*aBits1 & 0xFF) << 24);
#endif
		    ++aBits1;
		  }
	      }

	}

	if (anImage == NULL && ok) {
	  anImage = new ImageLib::Image(mImage->w, mImage->h);

	  Uint32 temp,pixel;
	  Uint8 red,green,blue,alpha;
	  SDL_Color color;
	  SDL_PixelFormat* fmt = mImage->format;

	  if (fmt->BytesPerPixel == 1) {
	    for(int i = 0; i < mImage->h; ++i) {
	      for (int j = 0; j < mImage->w; ++j) {
		Uint8 index = *(((Uint8*)mImage->pixels) + i*mImage->pitch + j);
		color=fmt->palette->colors[index]; 
		  if (mImage->flags & SDL_SRCCOLORKEY) {
		    if (SDL_MapRGB(fmt, color.r, color.g, color.b) == fmt->colorkey)
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 0;		 
		    else { 
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = color.r;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = color.g;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = color.b;          
		      *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
		    }
		  }   
		  else {
		    *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = color.r;          
		    *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = color.g;          
		    *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = color.b;          
		    *((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
		  }
	      }
	    }
	  }
	  else if (fmt->BytesPerPixel == 3) {
	    for(int i = 0; i < mImage->h; ++i) {
	      for (int j = 0; j < mImage->w; ++j) {

		Uint8* p = (((Uint8*)mImage->pixels) + i*mImage->pitch + j * 3);

#if __BIG_ENDIAN__
		pixel =  p[0] << 16 | p[1] << 8 | p[2]; 
#else
		pixel =  p[0] | p[1] << 8 | p[2] << 16; 
#endif
		/* Get Red component */ 
		temp=pixel&fmt->Rmask; /* Isolate red component */ 
		temp=temp>>fmt->Rshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Rloss; /* Expand to a full 8-bit number */ 
		red=(Uint8)temp; 
		/* Get Green component */ 
		temp=pixel&fmt->Gmask; /* Isolate green component */ 
		temp=temp>>fmt->Gshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Gloss; /* Expand to a full 8-bit number */ 
		green=(Uint8)temp; 
		/* Get Blue component */ 
		temp=pixel&fmt->Bmask; /* Isolate blue component */ 
		temp=temp>>fmt->Bshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Bloss; /* Expand to a full 8-bit number */ 
		blue=(Uint8)temp; 

		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = red;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = green;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = blue;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = 255;
	      }
	    }
	  }
	  else {
	    assert(fmt->BytesPerPixel == 4);
	    for(int i = 0; i < mImage->h; ++i) {
	      for (int j = 0; j < mImage->w; ++j) {

		pixel = *(((Uint32*)mImage->pixels) + i*(mImage->pitch/sizeof(Uint32)) + j);

		/* Get Red component */ 
		temp=pixel&fmt->Rmask; /* Isolate red component */ 
		temp=temp>>fmt->Rshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Rloss; /* Expand to a full 8-bit number */ 
		red=(Uint8)temp; 

		/* Get Green component */ 
		temp=pixel&fmt->Gmask; /* Isolate green component */ 
		temp=temp>>fmt->Gshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Gloss; /* Expand to a full 8-bit number */ 
		green=(Uint8)temp; 
		/* Get Blue component */ 
		temp=pixel&fmt->Bmask; /* Isolate blue component */ 
		temp=temp>>fmt->Bshift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Bloss; /* Expand to a full 8-bit number */ 
		blue=(Uint8)temp; 
		/* Get Alpha component */ 
		temp=pixel&fmt->Amask; /* Isolate alpha component */ 
		temp=temp>>fmt->Ashift;/* Shift it down to 8-bit */ 
		temp=temp<<fmt->Aloss; /* Expand to a full 8-bit number */ 
		alpha=(Uint8)temp; 

		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 2) = red;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 1) = green;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 0) = blue;          
		*((unsigned char*)anImage->mBits + (i * mImage->w + j) * sizeof(Uint32) + 3) = alpha;
	      }
	    }
	  }
	}

	if (mImage != NULL)
	  SDL_FreeSurface(mImage);

	return anImage;
}

