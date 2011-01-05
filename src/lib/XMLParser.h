#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__

#include <map>
#include <list>
#include <string>

#include "Common.h"

struct PFILE;           // Defined in PakInterface.h

namespace Sexy
{

typedef std::map<SexyString, SexyString>    XMLParamMap;
typedef std::list<XMLParamMap::iterator>    XMLParamMapIteratorList;

typedef std::vector<wchar_t> XMLParserBuffer;

class XMLElement
{
public:
    enum
    {
        TYPE_NONE,
        TYPE_START,
        TYPE_END,
        TYPE_ELEMENT,
        TYPE_INSTRUCTION,
        TYPE_COMMENT,
        TYPE_CDATA,
    };
public:

    bool                    hasAttribute(const SexyString & attr) const {return mAttributes.find(attr) != mAttributes.end(); }
    double                  attrFloatValue(const SexyString & attr, double default_value=0.0) const;
    int                     attrIntValue(const SexyString & attr, int default_value=0) const;

    int                     mType;
    SexyString              mSection;
    SexyString              mValue;
    SexyString              mInstruction;
    XMLParamMap             mAttributes;
    XMLParamMapIteratorList mAttributeIteratorList; // stores attribute iterators in their original order
};

class XMLParser
{
protected:
    std::string             mFileName;
    SexyString              mErrorText;
    int                     mLineNum;
    PFILE*                  mFile;
    bool                    mHasFailed;
    bool                    mAllowComments;
    XMLParserBuffer         mBufferedText;
    SexyString              mSection;
    bool                    (XMLParser::*mGetCharFunc)(wchar_t* theChar, bool* error);
    bool                    mForcedEncodingType;
    bool                    mFirstChar;
    bool                    mByteSwap;

protected:
    void                    Fail(const SexyString& theErrorText);
    void                    Init();

    bool                    AddAttribute(XMLElement* theElement, const SexyString& aAttributeKey, const SexyString& aAttributeValue);

    bool                    GetAsciiChar(wchar_t* theChar, bool* error);
    bool                    GetUTF8Char(wchar_t* theChar, bool* error);
    bool                    GetUTF16Char(wchar_t* theChar, bool* error);
    bool                    GetUTF16LEChar(wchar_t* theChar, bool* error);
    bool                    GetUTF16BEChar(wchar_t* theChar, bool* error);

public:
    enum XMLEncodingType
    {
        ASCII,
        UTF_8,
        UTF_16,
        UTF_16_LE,
        UTF_16_BE
    };

public:
    XMLParser();
    virtual ~XMLParser();

    void                    SetEncodingType(XMLEncodingType theEncoding);
    bool                    OpenFile(const std::string& theFilename);
    void                    SetStringSource(const std::wstring& theString);
    void                    SetStringSource(const std::string& theString);
    bool                    NextElement(XMLElement* theElement);
    SexyString              GetErrorText();
    int                     GetCurrentLineNum();
    std::string             GetFileName();

    inline void             AllowComments(bool doAllow) { mAllowComments = doAllow; }

    bool                    HasFailed();
    bool                    EndOfFile();
};

};

#endif //__XMLPARSER_H__

