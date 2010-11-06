{
        if (!(dx == 0 || dy == 0)) {
	PIXEL_TYPE* aDestPixels = &aBits[aY0*STRIDE + aX0];
	uint32_t anErrAcc = 0x10000 - (uint32_t)((1.0f - fabsf(theStartX - aX0))*(1.0f - fabsf(theStartY - aY0)) * 0x10000);
	if (dx >= dy) // mostly horizontal
	{
		//uint32_t anErrAdj = (((uint32_t)dy << 16) / (uint32_t)dx);
		uint32_t anErrAdj = (uint32_t)(dyd / dxd * 0x10000);

		uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
		uchar aWeight2 = (aWeight ^ 0xFF);

		int a = CALC_WEIGHT_A(aWeight);
		PIXEL_TYPE dest = *aDestPixels;
                if (a != 0) {
		    int oma = 256 - a;
		    BLEND_PIXEL(aDestPixels);
                }
		while (--dx)
		{
			anErrAcc += anErrAdj;
			if (anErrAcc >= 0x10000)
			{
				anErrAcc &= 0xFFFF;
				aDestPixels += STRIDE;
				aY0++;
			}
			aDestPixels += aXinc;

			uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
			uchar aWeight2 = (aWeight ^ 0xFF);

			dest = *aDestPixels;
			a = CALC_WEIGHT_A(aWeight);
                        if (a != 0) {
			    int oma = 256 - a;
			    BLEND_PIXEL(aDestPixels);
                        }
			if (aY0 < mHeight-1)
			{
				dest = *(aDestPixels+STRIDE);
				a = CALC_WEIGHT_A(aWeight2);
                                if (a != 0) {
                                    int oma = 256 - a;
				    BLEND_PIXEL(aDestPixels+STRIDE);
                                }
			}
		}
	}
	else // mostly vertical
	{
		//uint32_t anErrAdj = (uint32_t)(((uint32_t)dx << 16) / (uint32_t)dy);
		uint32_t anErrAdj = (uint32_t)(dxd / dyd * 0x10000);

		uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
		uchar aWeight2 = (aWeight ^ 0xFF);

		PIXEL_TYPE dest = *aDestPixels;
		int a = CALC_WEIGHT_A(aWeight);
                if (a != 0) {
		    int oma = 256 - a;
		    BLEND_PIXEL(aDestPixels);
                }
		while (--dy)
		{
			anErrAcc += anErrAdj;
			if (anErrAcc >= 0x10000)
			{
				anErrAcc &= 0xFFFF;
				aDestPixels += aXinc;
				aX0 += aXinc;
			}
			aDestPixels += STRIDE;

			uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
			uchar aWeight2 = (aWeight ^ 0xFF);

			dest = *aDestPixels;
			int a = CALC_WEIGHT_A(aWeight);
                        if (a != 0) {
                           int oma = 256 - a;
		 	   BLEND_PIXEL(aDestPixels);
                        }
			if (aX0 < STRIDE-1)
			{
				dest = *(aDestPixels+aXinc);
				a = CALC_WEIGHT_A(aWeight2);
                                if (a != 0) {
				    int oma = 256 - a;
				    BLEND_PIXEL(aDestPixels+aXinc);
                                }
			}
		}
	}
	aDestPixels = &aBits[aY1*STRIDE + aX1];
	uchar aWeight = 255 - (uchar)(anErrAcc >> 8);
	uchar aWeight2 = (aWeight ^ 0xFF);

	PIXEL_TYPE dest = *aDestPixels;
	int a = CALC_WEIGHT_A(aWeight);
        if (a != 0) {
	    int oma = 256 - a;
	    BLEND_PIXEL(aDestPixels);
        }
    }
}


