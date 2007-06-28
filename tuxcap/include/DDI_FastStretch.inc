{
	if (!LockSurface())
		return;	
	
	int aFPSrcY = theSrcRect.mY * 0x10000;

	int anAddX = (theSrcRect.mWidth * 0x10000) / theDestRect.mWidth;
	int anAddY = (theSrcRect.mHeight * 0x10000) / theDestRect.mHeight;

	if (theColor == Color::White)
	{
		if (mSurface->format->BitsPerPixel == 16)
		{
                //FIXME replace with Uint32 and Uint16
			ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + (theDestRect.mY * mSurface->pitch/2) + theDestRect.mX;

			ulong aRMask = mSurface->format->Rmask;
			ulong aGMask = mSurface->format->Gmask;
			ulong aBMask = mSurface->format->Bmask;

			ulong aRRoundAdd = aRMask >> 1;
			ulong aGRoundAdd = aGMask >> 1;
			ulong aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					ulong src = READ_COLOR(aSrcPixels);
					ulong dest = *aDestPixels;
					
					int a = src >> 24;	
					
					if (a != 0)
					{
						int oma = 256 - a;
						
						*(aDestPixels _PLUSPLUS) = 
							src +
							(((((dest & aRMask) * oma)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mSurface->pitch/2;
				aFPSrcY += anAddY;
			}		
		}
		else if (mSurface->format->BitsPerPixel == 32)
		{
			ulong* aDestPixelsRow = ((ulong*) mSurface->pixels) + (theDestRect.mY * mSurface->pitch/4) + theDestRect.mX;

			ulong aRMask = mSurface->format->Rmask;
			ulong aGMask = mSurface->format->Gmask;
			ulong aBMask = mSurface->format->Bmask;

			ulong aRRoundAdd = aRMask >> 1;
			ulong aGRoundAdd = aGMask >> 1;
			ulong aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ulong* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					ulong src = READ_COLOR(aSrcPixels);
					ulong dest = *aDestPixels;
					
					int a = src >> 24;	
					
					if (a != 0)
					{
						int oma = 256 - a;
						
						*(aDestPixels _PLUSPLUS) = 
							src +
							(((((dest & aRMask) * oma)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aFPSrcY += anAddY;
			}
		}
	}
	else
	{
		int ca = theColor.mAlpha;
		int cr = (theColor.mRed * ca) / 255;
		int cg = (theColor.mGreen * ca) / 255;
		int cb = (theColor.mBlue * ca) / 255;
	
		if (mSurface->format->BitsPerPixel == 16)
		{
			ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + (theDestRect.mY * mSurface->pitch/2) + theDestRect.mX;


			ulong aRMask = mSurface->format->Rmask;
			ulong aGMask = mSurface->format->Gmask;
			ulong aBMask = mSurface->format->Bmask;

			ulong aRRoundAdd = aRMask >> 1;
			ulong aGRoundAdd = aGMask >> 1;
			ulong aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ushort* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					ulong src = READ_COLOR(aSrcPixels);
					ulong dest = *aDestPixels;
					
					int a = ((src >> 24) * ca) / 255;
					
					if (a != 0)
					{
						int oma = 256 - a;
						
						*(aDestPixels _PLUSPLUS) = 
							(((((dest & aRMask) * oma) + ((src & aRMask) * cr)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma) + ((src & aGMask) * cg)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma) + ((src & aBMask) * cb)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mSurface->pitch/2;
				aFPSrcY += anAddY;
			}		
		}
		else if (mSurface->format->BitsPerPixel == 32)
		{
			ulong* aDestPixelsRow = ((ulong*) mSurface->pixels) + (theDestRect.mY * mSurface->pitch/4) + theDestRect.mX;


			ulong aRMask = mSurface->format->Rmask;
			ulong aGMask = mSurface->format->Gmask;
			ulong aBMask = mSurface->format->Bmask;

			ulong aRRoundAdd = aRMask >> 1;
			ulong aGRoundAdd = aGMask >> 1;
			ulong aBRoundAdd = aBMask >> 1;					
	
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				int aFPSrcX = theSrcRect.mX * 0x10000;

				ulong* aDestPixels = aDestPixelsRow;
				SRC_TYPE* aSrcPixelsRow = aSrcBits + (aSrcMemoryImage->mWidth * (aFPSrcY >> 16));

				for (int x = 0; x < theDestRect.mWidth; x++, aFPSrcX += anAddX)
				{
					SRC_TYPE* aSrcPixels = aSrcPixelsRow + (aFPSrcX >> 16);
					
					ulong src = READ_COLOR(aSrcPixels);
					ulong dest = *aDestPixels;
					
					int a = ((src >> 24) * ca) / 255;
					int oma = 256 - a;		
					
					if (a != 0)
					{
						int oma = 256 - a;
						
						*(aDestPixels _PLUSPLUS) = 
							(((((dest & aRMask) * oma) + ((src & aRMask) * cr)) >> 8) & aRMask) +
							(((((dest & aGMask) * oma) + ((src & aGMask) * cg)) >> 8) & aGMask) +
							(((((dest & aBMask) * oma) + ((src & aBMask) * cb)) >> 8) & aBMask);
					}
					else
						aDestPixels _PLUSPLUS;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aFPSrcY += anAddY;
			}
		}
	}
	
	UnlockSurface();
}

