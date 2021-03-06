{
	aMemoryImage->CommitBits();

//	RECT aDestRect = {theX, theY, theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
//	RECT aSrcRect = {theSrcRect.mX, theSrcRect.mY, theSrcRect.mX + theSrcRect.mWidth, theSrcRect.mY + theSrcRect.mHeight};

	if (!LockSurface())
		return;

	uchar* aSrcRLAdditiveData = aMemoryImage->GetRLAdditiveData(mDDInterface);

	if (mSurface->format->BitsPerPixel == 16)
	{
		ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + (theY * mSurface->pitch/2) + theX;

		uchar* aRLAdditiveDataRow = aSrcRLAdditiveData + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;

		uint32_t aRMask = mSurface->format->Rmask;
		uint32_t aGMask = mSurface->format->Gmask;
		uint32_t aBMask = mSurface->format->Bmask;

		//int aMaxR = (1<<mDDInterface->mRedBits) - 1;
		//int aMaxG = (1<<mDDInterface->mGreenBits) - 1;
		//int aMaxB = (1<<mDDInterface->mBlueBits) - 1;

		int aRedShift = mDDInterface->mRedShift;
		int aGreenShift = mDDInterface->mGreenShift;
		int aBlueShift = mDDInterface->mBlueShift;

		int* aMaxRedTable = mDDInterface->mRedAddTable;
		int* aMaxGreenTable = mDDInterface->mGreenAddTable;
		int* aMaxBlueTable = mDDInterface->mBlueAddTable;

		if (theColor == Color::White)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAdditiveData = aRLAdditiveDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAdditiveData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					if ((src & 0xFFFFFF) == 0) // A run of black
					{
						aSrcPixels += rl;
						aDestPixels _PLUSEQUALS rl;
					}
					else
					{
						for (int i = 0; i < rl; i++)
						{
							src = NEXT_SRC_COLOR;
							uint32_t dest = *aDestPixels;

							int r = aMaxRedTable[((dest & aRMask) + (src & aRMask)) >> aRedShift];
							int g = aMaxGreenTable[((dest & aGMask) + (src & aGMask)) >> aGreenShift];
							int b = aMaxBlueTable[((dest & aBMask) + (src & aBMask)) >> aBlueShift];

							*(aDestPixels _PLUSPLUS) =
								(r << aRedShift) |
								(g << aGreenShift) |
								(b << aBlueShift);
						}
					}

					aRLAdditiveData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mSurface->pitch/2;
				aSrcPixelsRow += theImage->GetWidth();
				aRLAdditiveDataRow += theImage->GetWidth();
			}
		}
		else
		{
			int ca = theColor.mAlpha;
			int cr = (theColor.mRed * ca) / 255;
			int cg = (theColor.mGreen * ca) / 255;
			int cb = (theColor.mBlue * ca) / 255;

			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				ushort* aDestPixels = aDestPixelsRow;
				uchar* aRLAdditiveData = aRLAdditiveDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAdditiveData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					if ((src & 0xFFFFFF) == 0) // A run of black
					{
						aSrcPixels += rl;
						aDestPixels _PLUSEQUALS rl;
					}
					else
					{
						for (int i = 0; i < rl; i++)
						{
							src = NEXT_SRC_COLOR;
							uint32_t dest = *aDestPixels;

							int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + ((((src & aRMask) >> aRedShift) * cr) >> 8)];
							int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + ((((src & aGMask) >> aGreenShift) * cg) >> 8)];
							int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + ((((src & aBMask) >> aBlueShift) * cb) >> 8)];

							*(aDestPixels _PLUSPLUS) =
								(r << aRedShift) |
								(g << aGreenShift) |
								(b << aBlueShift);
						}
					}

					aRLAdditiveData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mSurface->pitch/2;
				aSrcPixelsRow += theImage->GetWidth();
				aRLAdditiveDataRow += theImage->GetWidth();
			}
		}
	}
	else if (mSurface->format->BitsPerPixel == 32)
	{
		uint32_t* aDestPixelsRow = ((uint32_t*) mSurface->pixels) + (theY * mSurface->pitch/4) + theX;
		uchar* aRLAdditiveDataRow = aSrcRLAdditiveData + (theSrcRect.mY * theImage->GetWidth()) + theSrcRect.mX;

                uint32_t aRMask = mSurface->format->Rmask;
		uint32_t aGMask = mSurface->format->Gmask;
		uint32_t aBMask = mSurface->format->Bmask;

		//int aMaxR = (1<<mDDInterface->mRedBits) - 1;
		//int aMaxG = (1<<mDDInterface->mGreenBits) - 1;
		//int aMaxB = (1<<mDDInterface->mBlueBits) - 1;

		int aRedShift = mDDInterface->mRedShift;
		int aGreenShift = mDDInterface->mGreenShift;
		int aBlueShift = mDDInterface->mBlueShift;

		int* aMaxRedTable = mDDInterface->mRedAddTable;
		int* aMaxGreenTable = mDDInterface->mGreenAddTable;
		int* aMaxBlueTable = mDDInterface->mBlueAddTable;

		if (theColor == Color::White)
		{
			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				uchar* aRLAdditiveData = aRLAdditiveDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAdditiveData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					if ((src & 0xFFFFFF) == 0) // A run of black
					{
						aSrcPixels += rl;
						aDestPixels _PLUSEQUALS rl;
					}
					else
					{
						for (int i = 0; i < rl; i++)
						{
							src = NEXT_SRC_COLOR;
							uint32_t dest = *aDestPixels;

							int r = aMaxRedTable[((dest & aRMask) + (src & aRMask)) >> aRedShift];
							int g = aMaxGreenTable[((dest & aGMask) + (src & aGMask)) >> aGreenShift];
							int b = aMaxBlueTable[((dest & aBMask) + (src & aBMask)) >> aBlueShift];

							*(aDestPixels _PLUSPLUS) =
								(r << aRedShift) |
								(g << aGreenShift) |
								(b << aBlueShift);
						}
					}

					aRLAdditiveData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aSrcPixelsRow += theImage->GetWidth();
				aRLAdditiveDataRow += theImage->GetWidth();
			}
		}
		else
		{
			int ca = theColor.mAlpha;
			int cr = (theColor.mRed * ca) / 255;
			int cg = (theColor.mGreen * ca) / 255;
			int cb = (theColor.mBlue * ca) / 255;

			for (int y = 0; y < theSrcRect.mHeight; y++)
			{
				uint32_t* aDestPixels = aDestPixelsRow;
				uchar* aRLAdditiveData = aRLAdditiveDataRow;

				aSrcPixels = aSrcPixelsRow;

				for (int aSpanLeft = theSrcRect.mWidth; aSpanLeft > 0; )
				{
					uint32_t src = PEEK_SRC_COLOR;
					uchar rl = *aRLAdditiveData;

					if (rl > aSpanLeft)
						rl = aSpanLeft;

					if ((src & 0xFFFFFF) == 0) // A run of black
					{
						aSrcPixels += rl;
						aDestPixels _PLUSEQUALS rl;
					}
					else
					{
						for (int i = 0; i < rl; i++)
						{
							src = NEXT_SRC_COLOR;
							uint32_t dest = *aDestPixels;

							int r = aMaxRedTable[((dest & aRMask) >> aRedShift) + ((((src & aRMask) >> aRedShift) * cr) >> 8)];
							int g = aMaxGreenTable[((dest & aGMask) >> aGreenShift) + ((((src & aGMask) >> aGreenShift) * cg) >> 8)];
							int b = aMaxBlueTable[((dest & aBMask) >> aBlueShift) + ((((src & aBMask) >> aBlueShift) * cb) >> 8)];

							*(aDestPixels _PLUSPLUS) =
								(r << aRedShift) |
								(g << aGreenShift) |
								(b << aBlueShift);
						}
					}

					aRLAdditiveData += rl;
					aSpanLeft -= rl;
				}

				aDestPixelsRow += mSurface->pitch/4;
				aSrcPixelsRow += theImage->GetWidth();
				aRLAdditiveDataRow += theImage->GetWidth();
			}
		}
	}

	UnlockSurface();
}

