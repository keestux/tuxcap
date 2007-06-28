{	
	if (mSurface->format->BitsPerPixel == 16)
	{
		ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + (theY * mSurface->pitch/2) + theX;		

		if (aMemoryImage->mHasTrans)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;								
				
				aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					ulong aSrc = NEXT_SRC_COLOR;

					if (aSrc>>24 != 0)
						*(aDestPixels++) = (ushort) aSrc;
					else
						aDestPixels++;
				}

				aDestPixelsRow += mSurface->pitch/2;
				aSrcPixelsRow += theImage->mWidth;				
			}
		}
		else
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				
				aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					*(aDestPixels++) = (ushort) NEXT_SRC_COLOR;							
				}

				aDestPixelsRow += mSurface->pitch/2;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}
	else if (mSurface->format->BitsPerPixel == 32)
	{
		ulong* aDestPixelsRow = ((ulong*) mSurface->pixels) + (theY * mSurface->pitch/4) + theX;		

		if (aMemoryImage->mHasTrans)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ulong* aDestPixels = aDestPixelsRow;
				
				aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					ulong aSrc = NEXT_SRC_COLOR;

					if (aSrc>>24 != 0)
						*(aDestPixels++) = aSrc;
					else
						aDestPixels++;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
		else
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ulong* aDestPixels = aDestPixelsRow;
				
				aSrcPixels = aSrcPixelsRow;

				for (int x = 0; x < theSrcRect.mWidth; x++)
				{
					*(aDestPixels++) = NEXT_SRC_COLOR;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aSrcPixelsRow += theImage->mWidth;
			}
		}
	}

	UnlockSurface();
}

