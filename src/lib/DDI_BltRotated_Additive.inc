{
        double aCos = cos(theRot);
        double aSin = sin(theRot);

        int aCosLong = (int) (aCos * 0x10000);
        int aSinLong = (int) (aSin * 0x10000);

        if (!LockSurface())
                return;

        int* aMaxRedTable = mDDInterface->mRedAddTable;
        int* aMaxGreenTable = mDDInterface->mGreenAddTable;
        int* aMaxBlueTable = mDDInterface->mBlueAddTable;

        int aRedShift = mDDInterface->mRedShift;
        int aGreenShift = mDDInterface->mGreenShift;
        int aBlueShift = mDDInterface->mBlueShift;

        int aRedRightShift = 32 - mDDInterface->mRedBits;
        int aGreenRightShift = 24 - mDDInterface->mGreenBits;
        int aBlueRightShift = 16 - mDDInterface->mBlueBits;

        int aNumBits = mSurface->format->BitsPerPixel;

        uint32_t aRMask = mSurface->format->Rmask;
        uint32_t aGMask = mSurface->format->Gmask;
        uint32_t aBMask = mSurface->format->Bmask;

        //uint32_t aRRoundAdd = aRMask >> 1;
        //uint32_t aGRoundAdd = aGMask >> 1;
        //uint32_t aBRoundAdd = aBMask >> 1;

        if (aNumBits == 16)
        {
                ushort* aDestPixelsRow = ((ushort*) mSurface->pixels) + ((int)aDestRect.mY * mSurface->pitch/2) + (int)aDestRect.mX;
                int aDestPixelsPitch = mSurface->pitch/2;

                //TODO: if (theColor == Color::White)
                {
                        int ca = theColor.mAlpha;
                        int cr = (theColor.mRed * ca) / 255;
                        int cg = (theColor.mGreen * ca) / 255;
                        int cb = (theColor.mBlue * ca) / 255;

#                       define DEST_PIXEL_TYPE ushort
#                       define WRITE_PIXEL\
                        {\
                                ushort dest = *aDestPixels;\
                                \
                                int r = aMaxRedTable[((dest & aRMask)>>aRedShift) + \
                                        ((((((src1 & 0xFF0000) * a1) + \
                                            ((src2 & 0xFF0000) * a2) + \
                                            ((src3 & 0xFF0000) * a3) + \
                                            ((src4 & 0xFF0000) * a4)) >> aRedRightShift) * cr) >> 8)];\
                                int g = aMaxGreenTable[((dest & aGMask)>>aGreenShift) + \
                                        (((((src1 & 0x00FF00) * a1) + \
                                            ((src2 & 0x00FF00) * a2) + \
                                            ((src3 & 0x00FF00) * a3) + \
                                            ((src4 & 0x00FF00) * a4)) * cg) >> (aGreenRightShift + 8))];\
                                int b = aMaxBlueTable[((dest & aBMask)>>aBlueShift) + \
                                        (((((src1 & 0x0000FF) * a1) + \
                                            ((src2 & 0x0000FF) * a2) + \
                                            ((src3 & 0x0000FF) * a3) + \
                                            ((src4 & 0x0000FF) * a4)) * cb) >> (aBlueRightShift + 8))];\
                                \
                                *(aDestPixels++) = \
                                        (r << aRedShift) | \
                                        (g << aGreenShift) | \
                                        (b << aBlueShift); \
                        }
#                       include "BltRotatedHelper.inc"

#                       undef WRITE_PIXEL
#                       undef DEST_PIXEL_TYPE
                }
        }
        else if (aNumBits == 32)
        {
                uint32_t* aDestPixelsRow = ((uint32_t*) mSurface->pixels) + ((int)aDestRect.mY * mSurface->pitch/4) + (int)aDestRect.mX;
                int aDestPixelsPitch = mSurface->pitch/4;

                //if (theColor == Color::White)
                {
                        int ca = theColor.mAlpha;
                        int cr = (theColor.mRed * ca) / 255;
                        int cg = (theColor.mGreen * ca) / 255;
                        int cb = (theColor.mBlue * ca) / 255;


#                       define DEST_PIXEL_TYPE uint32_t
#                       define WRITE_PIXEL\
                        {\
                                uint32_t dest = *aDestPixels;\
                                \
                                int r = aMaxRedTable[((dest & aRMask)>>aRedShift) + \
                                        ((((((src1 & 0xFF0000) * a1) + \
                                            ((src2 & 0xFF0000) * a2) + \
                                            ((src3 & 0xFF0000) * a3) + \
                                            ((src4 & 0xFF0000) * a4)) >> aRedRightShift) * cr) >> 8)];\
                                int g = aMaxGreenTable[((dest & aGMask)>>aGreenShift) + \
                                        (((((src1 & 0x00FF00) * a1) + \
                                            ((src2 & 0x00FF00) * a2) + \
                                            ((src3 & 0x00FF00) * a3) + \
                                            ((src4 & 0x00FF00) * a4)) * cg) >> (aGreenRightShift + 8))];\
                                int b = aMaxBlueTable[((dest & aBMask)>>aBlueShift) + \
                                        (((((src1 & 0x0000FF) * a1) + \
                                            ((src2 & 0x0000FF) * a2) + \
                                            ((src3 & 0x0000FF) * a3) + \
                                            ((src4 & 0x0000FF) * a4)) * cb) >> (aBlueRightShift + 8))];\
                                \
                                *(aDestPixels++) = \
                                        (r << aRedShift) | \
                                        (g << aGreenShift) | \
                                        (b << aBlueShift);\
                        }

#                       include "BltRotatedHelper.inc"

#                       undef WRITE_PIXEL
#                       undef DEST_PIXEL_TYPE


                }
        }

        UnlockSurface();
}

