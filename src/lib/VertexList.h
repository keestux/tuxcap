/* 
 * File:   VertexList.h
 * Author: kees
 *
 * Created on March 23, 2011, 10:32 PM
 */

#ifndef VERTEXLIST_H
#define	VERTEXLIST_H

#include "D3DInterface.h"

namespace Sexy
{

struct VertexList {

    enum {
        MAX_STACK_VERTS = 100
    };
    D3DTLVERTEX mStackVerts[MAX_STACK_VERTS];
    D3DTLVERTEX *mVerts;
    int mSize;
    int mCapacity;

    typedef int size_type;

    VertexList() : mVerts(mStackVerts), mSize(0), mCapacity(MAX_STACK_VERTS)
    {
    }

    VertexList(const VertexList & theList) : mVerts(mStackVerts), mSize(theList.mSize), mCapacity(MAX_STACK_VERTS)
    {
        reserve(mSize);
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    ~VertexList()
    {
        if (mVerts != mStackVerts)
            delete mVerts;
    }

    void reserve(int theCapacity)
    {
        if (mCapacity < theCapacity) {
            mCapacity = theCapacity;
            D3DTLVERTEX *aNewList = new D3DTLVERTEX[theCapacity];
            memcpy(aNewList, mVerts, mSize * sizeof (mVerts[0]));
            if (mVerts != mStackVerts)
                delete mVerts;

            mVerts = aNewList;
        }
    }

    void push_back(const D3DTLVERTEX & theVert)
    {
        if (mSize == mCapacity)
            reserve(mCapacity * 2);

        mVerts[mSize++] = theVert;
    }

    void operator=(const VertexList & theList)
    {
        reserve(theList.mSize);
        mSize = theList.mSize;
        memcpy(mVerts, theList.mVerts, mSize * sizeof (mVerts[0]));
    }

    D3DTLVERTEX & operator[](int thePos)
    {
        return mVerts[thePos];
    }

    int size()
    {
        return mSize;
    }

    void clear()
    {
        mSize = 0;
    }

    void DoPolyTextureClip();
    void DrawPolyClipped(const Rect *theClipRect) const;
};

}

#endif	/* VERTEXLIST_H */

