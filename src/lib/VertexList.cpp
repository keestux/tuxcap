#include "VertexList.h"

using namespace Sexy;

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

static inline float GetCoord(const D3DTLVERTEX &theVertex, int theCoord)
{
    switch (theCoord) {
    case 0: return theVertex.sx;
    case 1: return theVertex.sy;
    case 3: return theVertex.tu;
    case 4: return theVertex.tv;
    default: return 0;
    }
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

void VertexList::DoPolyTextureClip()
{
    VertexList l2;

    float left = 0;
    float right = 1;
    float top = 0;
    float bottom = 1;

    VertexList *in = this, *out = &l2;
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

void VertexList::DrawPolyClipped(const Rect *theClipRect) const
{
    VertexList l1, l2;
    l1 = *this;

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
