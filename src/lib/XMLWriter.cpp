// XMLWriter.cpp: implementation of the XMLWriter class by James Poag
//
//////////////////////////////////////////////////////////////////////

#include "XMLWriter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace Sexy;

XMLWriter::XMLWriter()
{
	mFile = NULL;
	mLineNum = 0;
	mAllowComments = false;
	mOpenAttributes = false;
}

XMLWriter::~XMLWriter()
{
	if (mFile != NULL)
		fclose(mFile);
}

void XMLWriter::Fail(const std::string& theErrorText)
{
	mHasFailed = true;
	mErrorText = theErrorText;
}

void XMLWriter::Warn(const std::string &theWarning)
{
	mWarningStack.push("WARNING: " + theWarning);
}

void XMLWriter::Comment(const std::string &theComment)
{
	mWarningStack.push(theComment);
}

void XMLWriter::Init()
{
	mLineNum = 1;
	mHasFailed = false;
	mErrorText = "";	
}

bool XMLWriter::OpenFile(const std::string& theFileName)
{		
	mFile = fopen(theFileName.c_str(), "w");

	if (mFile == NULL)
	{
		mLineNum = 0;
		Fail("Unable to open file " + theFileName);		
		return false;
	}

	mFileName = theFileName.c_str();
	Init();

	//write header
	fprintf(mFile, "<?xml version=\"1.0\" ?>\n");
	mLineNum++;

	return true;
}

bool XMLWriter::HasFailed()
{
	return mHasFailed;
}

std::string XMLWriter::GetErrorText()
{
	return mErrorText;
}

int XMLWriter::GetCurrentLineNum()
{
	return mLineNum;
}

std::string XMLWriter::GetFileName()
{
	return mFileName;
}

/*
	Used as a tool to add attributes to XMLElement Nodes.
*/
bool XMLWriter::AddAttribute(XMLElement* theElement, const std::string& theAttributeKey, const std::string& theAttributeValue)
{
	std::pair<XMLParamMap::iterator,bool> aRet;

	aRet = theElement->mAttributes.insert(XMLParamMap::value_type(theAttributeKey, theAttributeValue));
	if (!aRet.second)
		aRet.first->second = theAttributeValue;

	if (theAttributeKey != "/")
		theElement->mAttributeIteratorList.push_back(aRet.first);

	return aRet.second;
}

/*
	Pushes the Element onto the section stack and creates 
	a new Node with an Attributes section
*/
bool XMLWriter::StartElement(const std::string &theElementName)
{
	CheckFileOpen();
	if(mHasFailed) return false;

	if(mOpenAttributes)
	{
		// Close Previous Element Attribute section
		fprintf(mFile, ">\n");
		mLineNum++;
	}

	if(!ValidateElementNodeName(theElementName))
	{
		Warn(theElementName + " is an Invalid Node Name.");
	}

	while(!mWarningStack.empty())
	{
		fprintf(mFile, "<!--  %s -->\n", mWarningStack.top().c_str());
		mWarningStack.pop();
	}

	mSectionStack.push(theElementName);
	
	for(unsigned int i = 1; i < mSectionStack.size(); i++)
	{
		fprintf(mFile, "\t");
	}
	
	fprintf(mFile, "<%s", theElementName.c_str());
	
	mOpenAttributes = true;
	return true;
}

/*
	Pushes theElement->Value onto the stack and adds all of 
	theElements->mAttributes to the Attribute section
*/
bool XMLWriter::StartElement(XMLElement *theElement)
{
	if(StartElement(theElement->mValue.c_str()) == false)
		return false;

	std::map<std::string, std::string>::iterator map_itr;
	map_itr = theElement->mAttributes.begin();

	for( ; map_itr != theElement->mAttributes.end(); map_itr++)
	{
		if(!WriteAttribute(map_itr->first, map_itr->second))
			return false;
	}
	
	return true;
}

/*
	Closes the previously open Node and pops it from the stack.
	Also Closes the Attributes Writing until StartElement is called.
*/
bool XMLWriter::StopElement()
{
	CheckFileOpen();
	if(mHasFailed) return false;

	if(mSectionStack.empty())
	{
		Fail("Stop Element Calls do not match StartElement Calls.");
		return false;
	}

	const std::string aNodeName = mSectionStack.top();
	mSectionStack.pop();

	if(mOpenAttributes)
	{
		// Close Previous Element Attribute section
		fprintf(mFile, "/>\n");
		mLineNum++;
	}
	else
	{
		// Otherwise close element section
		for(unsigned int i = 0; i < mSectionStack.size(); i++)
		{
			fprintf(mFile, "\t");
		}

		fprintf(mFile, "</%s>\n", aNodeName.c_str());
	}

	mOpenAttributes = false;

	while(!mWarningStack.empty())
	{
		fprintf(mFile, "<!--  %s -->\n", mWarningStack.top().c_str());
		mWarningStack.pop();
	}

	return true;
}

/*
	Adds an attribute to the Current Element.  If No element is open, then it returns
	false.
*/
bool XMLWriter::WriteAttribute(const std::string& aAttributeKey, const std::string& aAttributeValue)
{
	CheckFileOpen();
	if(mHasFailed) return false;

	if(mOpenAttributes)
	{
		if(!ValidateElementNodeName(aAttributeKey))
		{
			Warn(aAttributeKey + " is an invalid Attribute Name.");
		}
		
		fprintf(mFile, " %s=\"%s\"", aAttributeKey.c_str(), XMLEncodeString(aAttributeValue).c_str());
		return true;
	}

	if(mSectionStack.size())
		Fail("Attributes Section already closed for " + mSectionStack.top());
	else
		Fail("No Element Nodes Open for Writing Attributes.");

	return false;
}

bool XMLWriter::WriteAttribute(const std::string &aAttributeKey, const int &aAttributeValue)
{
	return WriteAttribute(aAttributeKey, StrFormat("%d", aAttributeValue));
}

bool XMLWriter::WriteAttribute(const std::string &aAttributeKey, const float &aAttributeValue)
{
	return WriteAttribute(aAttributeKey, StrFormat("%f", aAttributeValue));
}

bool XMLWriter::CloseFile()
{
	while(!mSectionStack.empty())
	{
		StopElement();
	}

	if (mFile != NULL)
	{
		fclose(mFile);
                mFile = NULL;
		return true;
	}

	Fail("File not Open");
	return false;
}

bool XMLWriter::ValidateElementNodeName(const std::string &theNodeName)
{
	const char* aNodeName = theNodeName.c_str();

	for(unsigned int i = 0; i < theNodeName.size(); i++)
	{
		if( aNodeName[i] < '0'
			|| (aNodeName[i] > '9' && aNodeName[i] < 'A')
			|| (aNodeName[i] > 'Z' && aNodeName[i] < '_')
			|| (aNodeName[i] > '_' && aNodeName[i] < 'a')
			|| aNodeName[i] > 'z')
		{
			return false;
		}
	}
	return true;
}

bool XMLWriter::CheckFileOpen()
{
	if (mFile != NULL)
	{
		return true;
	}

	Fail("No File Opened for writing");
	return false;
}


