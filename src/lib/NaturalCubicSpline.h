///////////////////////////////////////////////////////////////
//
// Written By: James Poag
//
// http://sexyfaq.jamespoag.com
//
///////////////////////////////////////////////////////////////
//
// Cubic Class, RegenerateSpline, and RegenrateClosedSpline
//  written by Tim Lambert.
//
// http://www.cse.unsw.edu.au/~lambert/
//
// Thanks to Brian "Ace" Rothstein For pointing me to Tim.
//
// Special Thanks to Paul Hamilton.
// http://www.mooktowngames.com
// http://mooktown.blogspot.com
//
///////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Usage
//////////////////////////////////////////////////////////////
//
// Use AddPoint() to create a Spline. 3 or more points are
// required to create a cubic spline.
//
// Think of the spline as a long straight line.  Addressing
// a Point on the line with a single distance variable. The
// Spline will return 2D coordinates for the position passed
// into 'GetPointAt'.  'GetTangentAt' reports the direction
// of travel.  Use 'atan2' to turn the tangent into a rotation.
//
// The rest of the functions are used in creating and refining
// the Spline in an editor
//
//////////////////////////////////////////////////////////////

#include <vector>
#include "Point.h"

namespace Sexy
{
    class Graphics;
    class XMLWriter;
    class XMLElement;
    class XMLParser;
};

class Cubic
{
    float a,b,c,d;         /* a + b*u + c*u^2 +d*u^3 */

public:
    Cubic(float a, float b, float c, float d){
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }
    ~Cubic(){};

    /** evaluate cubic */
public:
    float eval(float u) {
        return (((d*u) + c)*u + b)*u + a;
    }

    float tangent(float u){
        return ((3*d*u) + 2*c)*u + b;
    }
};
class NaturalCubicSpline
{
protected:
    std::vector<Cubic>          mYCubics;
    std::vector<Cubic>          mXCubics;
    std::vector<float>          mXCoords;
    std::vector<float>          mYCoords;

    std::vector<Sexy::Point>    mPoints;
    std::vector<Sexy::FPoint>   mSpline;

    std::vector<float>          mSplineSegmentLengths;

    virtual void RegenerateSpline(std::vector<float>& theInput, std::vector<Cubic>& theOutput);
    virtual void RegenerateClosedSpline(std::vector<float>& theInput, std::vector<Cubic>& theOutput);
    virtual float GetMinUFromLineAB(Sexy::FPoint A, Sexy::FPoint B, Sexy::Point C);
    virtual float GetMinDistanceFromLineAB(Sexy::FPoint A, Sexy::FPoint B, Sexy::Point C);

    float           mArcLength;
    unsigned int    mGranularity;

    bool            mClosed;

public:
    NaturalCubicSpline(void);
    virtual ~NaturalCubicSpline(void);

    // Drawing Functions
    virtual void                Draw(Sexy::Graphics* g); //Example on how to draw
    virtual void                DrawControlPoint(Sexy::Graphics* g, int theControlPointId, int theWidth);
    virtual void                DrawSplineSegment(Sexy::Graphics* g, int theSplineSegmentId);

    // The main functions
    virtual void                AddPoint(Sexy::Point thePoint);
    virtual void                RegenerateSplines();    // You Shouldn't have to call this.
    virtual Sexy::Point         GetPointAt(float theDistanceOnTheSpline);
    virtual Sexy::FPoint        GetTangentAt(float theLength);

    // ADT functions
    virtual int                 GetNumControlPoints(){return (int)mPoints.size();};
    virtual int                 GetNumSplineSegments(){return (int)mXCubics.size();};
    virtual float               GetArcLength(){return mArcLength;};
    virtual void                SetClosed(bool bClosed){mClosed = bClosed; RegenerateSplines();};
    virtual bool                isClosed(){return mClosed;};
    virtual int                 GetGranularity(){return mGranularity;};
    virtual void                SetGranularity(int theGranularity){mGranularity = theGranularity; RegenerateSplines();};

    // For Curve Refinement
    virtual void                BisectSegment(int theSplineSegmentId);
    virtual void                DeleteControlPoint(int theControlPointId);
    virtual void                ClearAllPoints();

    // Picking Helper Functions
    virtual float               GetClosestPointOnSegmentToPoint(Sexy::Point thePoint);
    virtual int                 GetControlPointIdNear(Sexy::Point thePoint);
    virtual int                 GetSegmentIdNear(Sexy::Point thePoint);
    virtual Sexy::Point         GetControlPoint(int theControlPointId);
    virtual void                SetControlPoint(int theControlPointId, Sexy::Point thePoint);

    //Serialization
    virtual void                Serialize(Sexy::XMLWriter* theWriter);
    virtual void                Serialize(Sexy::XMLParser* theParser, Sexy::XMLElement* theNode);

    virtual void                SaveToFile(std::string theFileName);
    virtual void                OpenFile(std::string theFileName);
};
