#include "NaturalCubicSpline.h"
#include "Graphics.h"
#include "SexyVector.h"
#include "XMLParser.h"
#include "XMLWriter.h"

#include <stack>
#include <cfloat>
using namespace Sexy;

NaturalCubicSpline::NaturalCubicSpline(void)
{
	mArcLength = 0.0f;
	mGranularity = 24;
	mClosed = false;
}

NaturalCubicSpline::~NaturalCubicSpline(void)
{
}

void NaturalCubicSpline::Draw(Graphics *g)
{
	g->SetColor(Color(0xaa,0xff, 0xaa));

	for(int i = 0; i < GetNumControlPoints(); i++)
	{
		DrawControlPoint(g, i, 4);
	}

	if(mSpline.size() > 2)
	{
		g->SetColor(Color(0xaa,0xaa, 0xff));

		for(int i = 0; i < GetNumSplineSegments(); i++)
		{
			DrawSplineSegment(g, i);
		}
	}
	else
	{
		g->SetColor(Color(0xaa,0xaa, 0xff));

		for(unsigned int i = 1; i < mPoints.size(); i++)
		{
			g->DrawLineAA(mPoints[i].mX, mPoints[i].mY, mPoints[i-1].mX, mPoints[i-1].mY);
		}
	}
}

void NaturalCubicSpline::DrawControlPoint(Sexy::Graphics *g, int theControlPointId, int theWidth)
{
	if(theControlPointId >= 0 && theControlPointId < (int)mPoints.size())
		g->FillRect(mPoints[theControlPointId].mX - theWidth/2, mPoints[theControlPointId].mY - theWidth/2, theWidth, theWidth);
}

void NaturalCubicSpline::DrawSplineSegment(Sexy::Graphics *g, int theSplineSegmentId)
{
	if(theSplineSegmentId >= 0 && theSplineSegmentId < (int)mXCubics.size())
	{
		for(unsigned int i = 0; i < mGranularity; i++)
		{
			int index = i + theSplineSegmentId*mGranularity + 1;
			g->DrawLineAA((int)mSpline[index].mX, (int)mSpline[index].mY, (int)mSpline[index-1].mX, (int)mSpline[index-1].mY);
		}
	}
}

void NaturalCubicSpline::AddPoint(Sexy::Point thePoint)
{
	mXCoords.push_back((float)thePoint.mX);
	mYCoords.push_back((float)thePoint.mY);

	mPoints.push_back(thePoint);
	RegenerateSplines();
}
// Returns the Tangent of a point on the line specified in Ribbon Coordinates.
// Tangents Tell the direction of travel and can be used to rotate the object to
// face the correct direction.  
Sexy::FPoint NaturalCubicSpline::GetTangentAt(float theLength)
{
	if(mXCubics.size() == 0) return Sexy::FPoint(0,0);
	if(mSpline.size() == 0) return Sexy::FPoint(0,0);
	if(theLength < 0.0f) return Sexy::FPoint(0,0);
	if(theLength > mArcLength) return Sexy::FPoint(0,0);

	float aDistance = theLength;
	unsigned int anIndex = 0;
	for(unsigned int i = 0; i < mSplineSegmentLengths.size(); i++, anIndex++)
	{
		aDistance -= mSplineSegmentLengths[i];
		if(aDistance < 0.0f)
		{
			aDistance += mSplineSegmentLengths[i];
			break;
		}
	}

	if(anIndex < mSplineSegmentLengths.size())
	{
		float u = aDistance/mSplineSegmentLengths[anIndex];
		Sexy::SexyVector2 aVector(mXCubics[anIndex].tangent(u), mYCubics[anIndex].tangent(u));
		aVector = aVector.Normalize();

		return Sexy::FPoint(aVector.x, aVector.y);
	}

	return Sexy::FPoint(0,0);
}

// Resolves a Distance on the spline to a Cartesian 2d Point on the Screen. 
// Exact position evaluated on the spline.
Sexy::Point NaturalCubicSpline::GetPointAt(float theDistanceOnTheSpline)
{
	if(mPoints.size() == 0) return Sexy::Point(0,0);
	if(mSpline.size() == 0) return mPoints.back();
	if(theDistanceOnTheSpline < 0.0f) return mPoints.front();
	if(theDistanceOnTheSpline > mArcLength) return mPoints.back();

	float aDistance = theDistanceOnTheSpline;
	unsigned int anIndex = 0;
	for(unsigned int i = 0; i < mSplineSegmentLengths.size(); i++, anIndex++)
	{
		aDistance -= mSplineSegmentLengths[i];
		if(aDistance < 0.0f)
		{
			aDistance += mSplineSegmentLengths[i];
			break;
		}
	}

	if(anIndex < mSplineSegmentLengths.size())
	{
		float u = aDistance/mSplineSegmentLengths[anIndex];
		return Sexy::Point((int)mXCubics[anIndex].eval(u), (int)mYCubics[anIndex].eval(u));
	}

	return mPoints.back();
}

// Creates and stores the Cubic Spline Data needed for all functions releated to Class.
// Requires 3 or more points caches the Splines as series of straight lines for faster operations
// such as picking and drawing.
void NaturalCubicSpline::RegenerateSplines()
{
	if(mClosed) //Full Circle
	{
		RegenerateClosedSpline(mXCoords, mXCubics);
		RegenerateClosedSpline(mYCoords, mYCubics);
	}
	else
	{
		RegenerateSpline(mXCoords, mXCubics);
		RegenerateSpline(mYCoords, mYCubics);
	}

	mSpline.clear();
	mSplineSegmentLengths.clear();
	if(mXCubics.size())
	{
		mSpline.push_back(Sexy::FPoint((float)mXCubics[0].eval(0), (float)mYCubics[0].eval(0)));
		for(unsigned int x = 0; x < mXCubics.size(); x++)
		{
			mSplineSegmentLengths.push_back(0.0f);

			for(unsigned int steps = 1; steps <= mGranularity; steps++)
			{
				float u = (float) steps / (float)mGranularity;
				mSpline.push_back(Sexy::FPoint((float)mXCubics[x].eval(u), (float)mYCubics[x].eval(u)));

				// Cache Segment Lengths
				unsigned int index = x*mGranularity + steps-1;
				mSplineSegmentLengths[x] += (float)sqrt( (((float)mSpline[index].mX - mSpline[index+1].mX)*((float)mSpline[index].mX - mSpline[index+1].mX))
				+ (((float)mSpline[index].mY - mSpline[index+1].mY)*((float)mSpline[index].mY - mSpline[index+1].mY)));
			}
		}

		mArcLength = 0.0f;
		for(unsigned int i = 1; i < mSpline.size(); i++)
			mArcLength += (float)sqrt( ((float)mSpline[i].mX - mSpline[i-1].mX)*((float)mSpline[i].mX - mSpline[i-1].mX)
				+ ((float)mSpline[i].mY - mSpline[i-1].mY)*((float)mSpline[i].mY - mSpline[i-1].mY));
	}	
}
// Generates the Spline with the assumption that the end is connected to the beginning
void NaturalCubicSpline::RegenerateClosedSpline(std::vector<float>& theInput, std::vector<Cubic>& theOutput)
{
	theOutput.clear();

	if(theInput.size() < 3) return;

	int n = (int)theInput.size() - 1;

	std::vector<float> w(n+1);
    std::vector<float> v(n+1);
    std::vector<float> y(n+1);
    std::vector<float> D(n+1);
    float z, F, G, H;
    int k;
    /* We solve the equation
       [4 1      1] [D[0]]   [3(theInput[1] - theInput[n])  ]
       |1 4 1     | |D[1]|   |3(theInput[2] - theInput[0])  |
       |  1 4 1   | | .  | = |      .         |
       |    ..... | | .  |   |      .         |
       |     1 4 1| | .  |   |3(theInput[n] - theInput[n-2])|
       [1      1 4] [D[n]]   [3(theInput[0] - theInput[n-1])]
       
       by decomposing the matrix into upper triangular and lower matrices
       and then back sustitution.  See Spath "Spline Algorithms for Curves
       and Surfaces" pp 19--21. The D[i] are the derivatives at the knots.
       */
    w[1] = v[1] = z = 1.0f/4.0f;
    y[0] = z * 3 * (theInput[1] - theInput[n]);
    H = 4;
    F = 3 * (theInput[0] - theInput[n-1]);
    G = 1;
    for ( k = 1; k < n; k++) {
      v[k+1] = z = 1/(4 - v[k]);
      w[k+1] = -z * w[k];
      y[k] = z * (3*(theInput[k+1]-theInput[k-1]) - y[k-1]);
      H = H - G * w[k];
      F = F - G * y[k-1];
      G = -v[k] * G;
    }
    H = H - (G+1)*(v[n]+w[n]);
    y[n] = F - (G+1)*y[n-1];
    
    D[n] = y[n]/H;
    D[n-1] = y[n-1] - (v[n]+w[n])*D[n]; /* This equation is WRONG! in my copy of Spath */
    for ( k = n-2; k >= 0; k--) {
      D[k] = y[k] - v[k+1]*D[k+1] - w[k+1]*D[n];
    }


    /* now compute the coefficients of the cubics */
    for ( k = 0; k < n; k++) {
		theOutput.push_back(Cubic((float)theInput[k], D[k], 3*(theInput[k+1] - theInput[k]) - 2*D[k] - D[k+1],
		       2*(theInput[k] - theInput[k+1]) + D[k] + D[k+1]));
    }
	theOutput.push_back(Cubic((float)theInput[n], D[n], 3*(theInput[0] - theInput[n]) - 2*D[n] - D[0],
		     2*(theInput[n] - theInput[0]) + D[n] + D[0]));
}

// Generates the Spline
void NaturalCubicSpline::RegenerateSpline(std::vector<float>& theInput, std::vector<Cubic>& theOutput)
{
	theOutput.clear();

	if(theInput.size() < 3) return;

	int n = (int)theInput.size() - 1;
	std::vector<float> gamma(n+1);
	std::vector<float> delta(n+1);
	std::vector<float> D(n+1);
	int i;
	/* We solve the equation
	[2 1       ] [D[0]]   [3(theInput[1] - theInput[0])  ]
	|1 4 1     | |D[1]|   |3(theInput[2] - theInput[0])  |
	|  1 4 1   | | .  | = |      .         |
	|    ..... | | .  |   |      .         |
	|     1 4 1| | .  |   |3(theInput[n] - theInput[n-2])|
	[       1 2] [D[n]]   [3(theInput[n] - theInput[n-1])]

	by using row operations to convert the matrix to upper triangular
	and then back sustitution.  The D[i] are the derivatives at the knots.
	*/

	gamma[0] = 1.0f/2.0f;
	for ( i = 1; i < n; i++) {
		gamma[i] = 1/(4-gamma[i-1]);
	}
	gamma[n] = 1/(2-gamma[n-1]);

	delta[0] = 3*(theInput[1]-theInput[0])*gamma[0];
	for ( i = 1; i < n; i++) {
		delta[i] = (3*(theInput[i+1]-theInput[i-1])-delta[i-1])*gamma[i];
	}
	delta[n] = (3*(theInput[n]-theInput[n-1])-delta[n-1])*gamma[n];

	D[n] = delta[n];
	for ( i = n-1; i >= 0; i--) {
		D[i] = delta[i] - gamma[i]*D[i+1];
	}

	/* now compute the coefficients of the cubics */
	for ( i = 0; i < n; i++) 
		theOutput.push_back(Cubic((float)theInput[i], D[i], 3*(theInput[i+1] - theInput[i]) - 2*D[i] - D[i+1],
			2*(theInput[i] - theInput[i+1]) + D[i] + D[i+1]));
	
}


// Returns the linear distance on the Spline Closest to the input Point
// This is computed by using the cached line segments (not exact solution)
float NaturalCubicSpline::GetClosestPointOnSegmentToPoint(Sexy::Point thePoint)
{
	if(mSpline.size())
	{
		float min_distance = FLT_MAX;// Something Large
		int min_index = -1;

		for(unsigned int i = 1; i < mSpline.size(); i++)
		{
			float temp_min_dist = (float)sqrt((thePoint.mX - mSpline[i].mX)*(thePoint.mX - mSpline[i].mX) + (thePoint.mY - mSpline[i].mY)*(thePoint.mY - mSpline[i].mY));
			if(temp_min_dist < min_distance)
			{
				min_distance = temp_min_dist;
				min_index = i;
			}
		}

		if(min_index > 0 && min_distance < 20.00f)
		{
			unsigned int min_cubic_index = (min_index - 1)/mGranularity;
			float min_u = GetMinUFromLineAB(mSpline[min_index-1], mSpline[min_index], thePoint);
			int step_mod = (min_index)%mGranularity;
			
			float ret_dist = 0.0f;

			for(unsigned int i = 0; i < min_cubic_index; i++)
			{
				ret_dist += mSplineSegmentLengths[i];
			}

			unsigned int start_segments = min_cubic_index*mGranularity;
			for(unsigned int i = start_segments + 1; i < start_segments+step_mod; i++)
			{
				ret_dist += (float)sqrt((mSpline[i].mX-mSpline[i-1].mX)*(mSpline[i].mX-mSpline[i-1].mX) 
					+ (mSpline[i].mY-mSpline[i-1].mY)*(mSpline[i].mY-mSpline[i-1].mY));
			}

			ret_dist += (float)min_u * (float)sqrt((mSpline[min_index].mX-mSpline[min_index-1].mX)*(mSpline[min_index].mX-mSpline[min_index-1].mX) 
				+ (mSpline[min_index].mY-mSpline[min_index-1].mY)*(mSpline[min_index].mY-mSpline[min_index-1].mY));

			return ret_dist;
		}
	}
	return 0.0f;
}

float NaturalCubicSpline::GetMinDistanceFromLineAB(Sexy::FPoint A, Sexy::FPoint B, Sexy::Point C)
{
	float Bx_Ax = (float)(B.mX - A.mX);
	float By_Ay = (float)(B.mY - A.mY);

	float u = GetMinUFromLineAB(A,B,C);

	float x = (float)(A.mX + u*Bx_Ax);
	float y = (float)(A.mY + u*By_Ay);

	return sqrt((C.mX - x)*(C.mX - x) + (C.mY - y)*(C.mY - y));
}

float NaturalCubicSpline::GetMinUFromLineAB(Sexy::FPoint A, Sexy::FPoint B, Sexy::Point C)
{
	float Bx_Ax = (float)(B.mX - A.mX);
	float Cx_Ax = (float)(C.mX - A.mX);
	float By_Ay = (float)(B.mY - A.mY);
	float Cy_Ay = (float)(C.mY - A.mY);

	float Bx_Ax_2 = (float)((Bx_Ax) * (Bx_Ax));
	float By_Ay_2 = (float)((By_Ay) * (By_Ay));

	return (Cx_Ax*Bx_Ax + Cy_Ay*By_Ay)/(Bx_Ax_2 + By_Ay_2);
}


// Returns -1 if no point founds
int NaturalCubicSpline::GetControlPointIdNear(Sexy::Point thePoint)
{
	float min_distance = FLT_MAX;
	int ret_index = -1;

	for(unsigned int i = 0; i < mPoints.size(); i++)
	{
		float dist = (float)sqrt((float)(thePoint.mX - mPoints[i].mX)*(thePoint.mX - mPoints[i].mX) + (thePoint.mY - mPoints[i].mY)*(thePoint.mY - mPoints[i].mY));

		if( dist < 20.0f && dist < min_distance)
		{
			min_distance = dist;
			ret_index = i;
		}
	}

	return ret_index;
}

// Returns -1 if No Segments Found
int NaturalCubicSpline::GetSegmentIdNear(Sexy::Point thePoint)
{
	float min_distance = FLT_MAX;// Something Large
	int min_index = -1;

	for(unsigned int i = 0; i < mSpline.size(); i++)
	{
		float temp_min_dist = (float)sqrt((thePoint.mX - mSpline[i].mX)*(thePoint.mX - mSpline[i].mX) + (thePoint.mY - mSpline[i].mY)*(thePoint.mY - mSpline[i].mY));
		if(temp_min_dist < min_distance)
		{
			min_distance = temp_min_dist;
			min_index = i;
		}
	}

	if(min_index >= 0 && min_distance < 20.00f)
	{
		return min_index/mGranularity;
	}

	return -1;

}

// Returns the Coordinates of the Control Point Specified by the id
Sexy::Point NaturalCubicSpline::GetControlPoint(int theControlPointId)
{
	if(theControlPointId < (int)mPoints.size())
		return mPoints[theControlPointId];

	return Sexy::Point(0,0);
}

// Used to Move Control Points and Adjust the Spline.  Use 'GetControlPointIdNear' to get a valid ID
void NaturalCubicSpline::SetControlPoint(int theControlPointId, Sexy::Point thePoint)
{
	if(theControlPointId >= 0 && theControlPointId < (int)mPoints.size())
	{
		mPoints[theControlPointId] = thePoint;
		mXCoords[theControlPointId] = (float)thePoint.mX;
		mYCoords[theControlPointId] = (float)thePoint.mY;

		RegenerateSplines();
	}
}

// Call this To Add a Control Point at the Mid-way point of the cubic
// addressed by theSplineSegmentId
void NaturalCubicSpline::BisectSegment(int theSplineSegmentId)
{
	if(theSplineSegmentId >= 0 && theSplineSegmentId < (int)mXCubics.size())
	{
		Sexy::Point aBisectingPoint((int)mXCubics[theSplineSegmentId].eval(0.5f), (int)mYCubics[theSplineSegmentId].eval(0.5f));

		std::stack<Sexy::Point> aStack;

		for(int i = (int)mPoints.size()-1; i > theSplineSegmentId; i--)
		{
			aStack.push(mPoints.back());
			mPoints.pop_back();
			mXCoords.pop_back();
			mYCoords.pop_back();
		}

		AddPoint(aBisectingPoint);

		while(!aStack.empty())
		{
			AddPoint(aStack.top());
			aStack.pop();
		}

		RegenerateSplines();
	}
}

void NaturalCubicSpline::DeleteControlPoint(int theControlPointId)
{
	if(theControlPointId >= 0 && theControlPointId < (int)mPoints.size())
	{
		std::stack<Sexy::Point> aStack;
		for(int i = (int)mPoints.size() - 1; i >= 0; i--)
		{
			if((int) i != theControlPointId)
				aStack.push(mPoints[i]);
		}

		ClearAllPoints();

		while(!aStack.empty())
		{
			AddPoint(aStack.top());
			aStack.pop();
		}

		RegenerateSplines();
	}
}

void NaturalCubicSpline::ClearAllPoints()
{
	mPoints.clear();
	mXCoords.clear();
	mYCoords.clear();

	mArcLength = 0.0f;

	RegenerateSplines();
}

// Parse an XML snippet named NaturalCubicSpline
void NaturalCubicSpline::Serialize(Sexy::XMLParser *theParser, Sexy::XMLElement *theNode)
{
	ClearAllPoints();

	if(theParser && theNode && !theParser->HasFailed())
	{
		if(theNode->mValue == "NaturalCubicSpline")
		{
			if(theNode->mAttributes.find("granularity") != theNode->mAttributes.end())
				mGranularity = sexyatoi(theNode->mAttributes["granularity"].c_str());

			if(theNode->mAttributes.find("isClosed") != theNode->mAttributes.end())
				mClosed = (theNode->mAttributes["isClosed"] == "true");
		}

		while(theParser->NextElement(theNode))
		{
			switch(theNode->mType)
			{
			case XMLElement::TYPE_START:
				{
					if(theNode->mValue == "Point")
					{
						if(theNode->mAttributes.find("x") != theNode->mAttributes.end() &&
							theNode->mAttributes.find("y") != theNode->mAttributes.end())
						{
							int x = 0,y = 0;
							x = sexyatoi(theNode->mAttributes["x"].c_str());
							y = sexyatoi(theNode->mAttributes["y"].c_str());

							AddPoint(Sexy::Point(x,y));
						}
					}
					else if(theNode->mValue == "NaturalCubicSpline")
					{
						if(theNode->mAttributes.find("granularity") != theNode->mAttributes.end())
							mGranularity = sexyatoi(theNode->mAttributes["granularity"].c_str());

						if(theNode->mAttributes.find("isClosed") != theNode->mAttributes.end())
							mClosed = (theNode->mAttributes["isClosed"] == "true");
					}
					break;
				}
			case XMLElement::TYPE_END:
				{
					if(theNode->mValue == "NaturalCubicSpline")
					{
						RegenerateSplines();
						return;
					}
					break;
				}
			}
		}
	}
		
	RegenerateSplines();
}

// XMLWriter class is included with this solution
void NaturalCubicSpline::Serialize(Sexy::XMLWriter *theWriter)
{
	if(theWriter)
	{
		theWriter->StartElement("NaturalCubicSpline");
		theWriter->WriteAttribute("isClosed", (mClosed)? "true" : "false");
		theWriter->WriteAttribute("granularity", (int)mGranularity);

		for(unsigned int i = 0; i < mPoints.size(); i++)
		{
			theWriter->StartElement("Point");
			theWriter->WriteAttribute("x", mPoints[i].mX);
			theWriter->WriteAttribute("y", mPoints[i].mY);
			theWriter->StopElement();
		}
		theWriter->StopElement();
	}
}

void NaturalCubicSpline::OpenFile(std::string theFileName)
{
	XMLParser aParser;

	if(aParser.OpenFile(theFileName) && !aParser.HasFailed())
	{
		XMLElement aNode;
		Serialize(&aParser, &aNode);
	}
}

void NaturalCubicSpline::SaveToFile(std::string theFileName)
{
	XMLWriter aWriter;
	
	if(aWriter.OpenFile(theFileName) && !aWriter.HasFailed())
		Serialize(&aWriter);

	aWriter.CloseFile();
}
