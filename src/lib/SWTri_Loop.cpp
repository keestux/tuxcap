// This file is included by SWTri.cpp and should not be built directly by the project.

    #if defined(MOD_ARGB) || defined(TEXTURED)
    int64_t subTex = x0 - lx;
    #endif

    #if defined(MOD_ARGB)
    unsigned int    r, g, b, a;
        a = la + static_cast<int>((da * subTex)>>16);
        r = lr + static_cast<int>((dr * subTex)>>16);
        g = lg + static_cast<int>((dg * subTex)>>16);
        b = lb + static_cast<int>((db * subTex)>>16);
    #endif

    #if defined(TEXTURED)
    unsigned int    u, v;
        u = lu + static_cast<int>((du * subTex)>>16);
        v = lv + static_cast<int>((dv * subTex)>>16);
    #endif

    PTYPE *     pix = fb + (x0>>16);
    int     width = ((x1-x0)>>16);
    
    while (width-- > 0)
    {
        // One of
        //   #define PIXEL_INCLUDE "SWTri_Pixel8888.cpp"
        //   #define PIXEL_INCLUDE "SWTri_Pixel888.cpp"
        //   #define PIXEL_INCLUDE "SWTri_Pixel656.cpp"
        //   #define PIXEL_INCLUDE "SWTri_Pixel555.cpp"
        #include PIXEL_INCLUDE
//      if (bit_format == 0x888) PIXEL888()
//      if (bit_format == 0x565) PIXEL565()
//      if (bit_format == 0x555) PIXEL555()
//      if (bit_format == 0x8888) PIXEL8888()
        ++pix;
        #if defined(MOD_ARGB)
            a += da;
            r += dr;
            g += dg;
            b += db;
        #endif

        #if defined(TEXTURED)
            u += du;
            v += dv;
        #endif
    }

    lx += ldx;
    sx += sdx;
    fb += pitch;

    #if defined (MOD_ARGB)
        la += lda;
        lr += ldr;
        lg += ldg;
        lb += ldb;
    #endif

    #if defined (TEXTURED)  
        lu += ldu;
        lv += ldv;
    #endif

