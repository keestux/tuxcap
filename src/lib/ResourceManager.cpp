
#include "ResourceManager.h"
#include "XMLParser.h"
#include "SoundManager.h"
#include "DDImage.h"
#include "Timer.h"
#if 0
#include "D3DInterface.h"
#include "SysFont.h"
//#define SEXY_PERF_ENABLED
#include "PerfTimer.h"
#endif

#include "ImageFont.h"
#include "ImageLib.h"

#include <memory>

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::ImageRes::DeleteResource()
{   
    mImage.Release();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SoundRes::DeleteResource()
{
    if (mSoundId >= 0)
        gSexyAppBase->mSoundManager->ReleaseSound(mSoundId);

    mSoundId = -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::FontRes::DeleteResource()
{
    delete mFont;
    mFont = NULL;

    delete mImage;
    mImage = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::ResourceManager(SexyAppBase *theApp) 
{
    mLogFacil = LoggerFacil::find("resman");
    Logger::tlog(mLogFacil, 1, "new ResourceManager");

    mApp = theApp;
    mHasFailed = false;
    mXMLParser = NULL;

    mAllowMissingProgramResources = false;
    mAllowAlreadyDefinedResources = false;
    mCurResGroupList = NULL;

    mLoadingResourcesStarted = false;
    mLoadingResourcesCompleted = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::~ResourceManager()
{
    DeleteMap(mImageMap);
    DeleteMap(mSoundMap);
    DeleteMap(mFontMap);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::IsGroupLoaded(const std::string &theGroup)
{
    return mLoadedGroups.find(theGroup)!=mLoadedGroups.end();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteMap(ResMap &theMap)
{
    for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
    {
        anItr->second->DeleteResource();
        delete anItr->second;
    }

    theMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(ResMap &theMap, const std::string &theGroup)
{
    for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
    {
        if (theGroup.empty() || anItr->second->mResGroup==theGroup)
            anItr->second->DeleteResource();
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(const std::string &theGroup)
{
    DeleteResources(mImageMap,theGroup);
    DeleteResources(mSoundMap,theGroup);
    DeleteResources(mFontMap,theGroup);
    mLoadedGroups.erase(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteExtraImageBuffers(const std::string &theGroup)
{
    for (ResMap::iterator anItr = mImageMap.begin(); anItr != mImageMap.end(); ++anItr)
    {
        if (theGroup.empty() || anItr->second->mResGroup==theGroup)
        {
            ImageRes *aRes = (ImageRes*)anItr->second;
            MemoryImage *anImage = (MemoryImage*)aRes->mImage;
            if (anImage != NULL)
                anImage->DeleteExtraBuffers();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string ResourceManager::GetErrorText()
{
    return mError;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::HadError()
{
    return mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::Fail(const std::string& theErrorText)
{
    if (!mHasFailed)
    {
        mHasFailed = true;
        if (mXMLParser==NULL)
        {
            mError = theErrorText;
            return false;
        }

        int aLineNum = mXMLParser->GetCurrentLineNum();

        char aLineNumStr[16];
        sprintf(aLineNumStr, "%d", aLineNum);   

        mError = theErrorText;

        if (aLineNum > 0)
            mError += std::string(" on Line ") + aLineNumStr;

        if (mXMLParser->GetFileName().length() > 0)
            mError += " in File '" + mXMLParser->GetFileName() + "'";
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseCommonResource(XMLElement &theElement, BaseRes *theRes, ResMap &theMap)
{
    mHadAlreadyDefinedError = false;

    const SexyString &aPath = theElement.mAttributes[_S("path")];
    if (aPath.empty())
        return Fail("No path specified.");

    theRes->mXMLAttributes = theElement.mAttributes;
    theRes->mFromProgram = false;
    if (aPath[0]==_S('!'))
    {
        theRes->mPath = SexyStringToStringFast(aPath);
        if (aPath==_S("!program"))
            theRes->mFromProgram = true;
    }
    else if (aPath[0]==_S('/'))
            theRes->mPath = SexyStringToStringFast(aPath);
    else
        theRes->mPath = mDefaultPath + SexyStringToStringFast(aPath);

    
    std::string anId;
    XMLParamMap::iterator anItr = theElement.mAttributes.find(_S("id"));
    if (anItr == theElement.mAttributes.end())
        anId = mDefaultIdPrefix + GetFileName(theRes->mPath,true);
    else
        anId = mDefaultIdPrefix + SexyStringToStringFast(anItr->second);

    theRes->mResGroup = mCurResGroup;
    theRes->mId = anId;

    std::pair<ResMap::iterator,bool> aRet = theMap.insert(ResMap::value_type(anId,theRes));
    if (!aRet.second)
    {
        mHadAlreadyDefinedError = true;
        return Fail("Resource already defined.");
    }

    mCurResGroupList->push_back(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSoundResource(XMLElement &theElement)
{
    SoundRes *aRes = new SoundRes;
    aRes->mSoundId = -1;
    aRes->mVolume = -1;
    aRes->mPanning = 0;

    if (!ParseCommonResource(theElement, aRes, mSoundMap))
    {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
        {
            mError = "";
            mHasFailed = false;
            SoundRes *oldRes = aRes;
            aRes = (SoundRes*)mSoundMap[oldRes->mId];
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        }
        else            
        {
            delete aRes;
            return false;
        }
    }
    
    XMLParamMap::iterator anItr;

    anItr = theElement.mAttributes.find(_S("volume"));
    if (anItr != theElement.mAttributes.end())
        sexysscanf(anItr->second.c_str(),_S("%lf"),&aRes->mVolume);

    anItr = theElement.mAttributes.find(_S("pan"));
    if (anItr != theElement.mAttributes.end())
        sexysscanf(anItr->second.c_str(),_S("%d"),&aRes->mPanning);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void ReadIntVector(const SexyString &theVal, std::vector<int> &theVector)
{
    theVector.clear();

    std::string::size_type aPos = 0;
    while (true)
    {
        theVector.push_back(sexyatoi(theVal.c_str()+aPos));
        aPos = theVal.find_first_of(_S(','),aPos);
        if (aPos==std::string::npos)
            break;

        aPos++;
    }   
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseImageResource(XMLElement &theElement)
{
    ImageRes *aRes = new ImageRes;
    if (!ParseCommonResource(theElement, aRes, mImageMap))
    {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
        {
            mError = "";
            mHasFailed = false;
            ImageRes *oldRes = aRes;
            aRes = (ImageRes*)mImageMap[oldRes->mId];
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        }
        else            
        {
            delete aRes;
            return false;
        }
    }

    aRes->mPalletize = !theElement.hasAttribute(_S("nopal"));
    aRes->mA4R4G4B4 = theElement.hasAttribute(_S("a4r4g4b4"));
    aRes->mDDSurface = theElement.hasAttribute(_S("ddsurface"));
    aRes->mPurgeBits = theElement.hasAttribute(_S("nobits")) ||
        (mApp->Is3DAccelerated() && theElement.hasAttribute(_S("nobits3d"))) ||
        (!mApp->Is3DAccelerated() && theElement.hasAttribute(_S("nobits2d")));
    aRes->mA8R8G8B8 = theElement.hasAttribute(_S("a8r8g8b8"));
    aRes->mMinimizeSubdivisions = theElement.hasAttribute(_S("minsubdivide"));
    aRes->mAutoFindAlpha = !theElement.hasAttribute(_S("noalpha"));

    XMLParamMap::iterator anItr;
    anItr = theElement.mAttributes.find(_S("alphaimage"));
    if (anItr != theElement.mAttributes.end())
        aRes->mAlphaImage = mDefaultPath + SexyStringToStringFast(anItr->second);

    aRes->mAlphaColor = 0xFFFFFF;
    anItr = theElement.mAttributes.find(_S("alphacolor"));
    if (anItr != theElement.mAttributes.end())
          sexysscanf(anItr->second.c_str(),_S("%x"),(unsigned int*)&aRes->mAlphaColor);

    if (theElement.hasAttribute(_S("variant"))) {
        Fail("'variant' not supported anymore");
        return false;
    }

    anItr = theElement.mAttributes.find(_S("alphagrid"));
    if (anItr != theElement.mAttributes.end())
        aRes->mAlphaGridImage = mDefaultPath + SexyStringToStringFast(anItr->second);

    anItr = theElement.mAttributes.find(_S("rows"));
    if (anItr != theElement.mAttributes.end())
        aRes->mRows = sexyatoi(anItr->second.c_str());
    else
        aRes->mRows = 1;

    anItr = theElement.mAttributes.find(_S("cols"));
    if (anItr != theElement.mAttributes.end())
        aRes->mCols = sexyatoi(anItr->second.c_str());
    else
        aRes->mCols = 1;

    anItr = theElement.mAttributes.find(_S("anim"));
    AnimType anAnimType = AnimType_None;
    if (anItr != theElement.mAttributes.end())
    {
        const SexyChar *aType = anItr->second.c_str();

        if (sexystricmp(aType,_S("none"))==0) anAnimType = AnimType_None;
        else if (sexystricmp(aType,_S("once"))==0) anAnimType = AnimType_Once;
        else if (sexystricmp(aType,_S("loop"))==0) anAnimType = AnimType_Loop;
        else if (sexystricmp(aType,_S("pingpong"))==0) anAnimType = AnimType_PingPong;
        else 
        {
            Fail("Invalid animation type.");
            return false;
        }
    }
    aRes->mAnimInfo.mAnimType = anAnimType;
    if (anAnimType != AnimType_None)
    {
          int aNumCels = std::max(aRes->mRows,aRes->mCols);
        int aBeginDelay = 0, anEndDelay = 0;

        anItr = theElement.mAttributes.find(_S("framedelay"));
        if (anItr != theElement.mAttributes.end())
            aRes->mAnimInfo.mFrameDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("begindelay"));
        if (anItr != theElement.mAttributes.end())
            aBeginDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("enddelay"));
        if (anItr != theElement.mAttributes.end())
            anEndDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("perframedelay"));
        if (anItr != theElement.mAttributes.end())
            ReadIntVector(anItr->second,aRes->mAnimInfo.mPerFrameDelay);

        anItr = theElement.mAttributes.find(_S("framemap"));
        if (anItr != theElement.mAttributes.end())
            ReadIntVector(anItr->second,aRes->mAnimInfo.mFrameMap);

        aRes->mAnimInfo.Compute(aNumCels,aBeginDelay,anEndDelay);
    }


    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseFontResource(XMLElement &theElement)
{
    FontRes *aRes = new FontRes;
    aRes->mFont = NULL;
    aRes->mImage = NULL;

    if (!ParseCommonResource(theElement, aRes, mFontMap))
    {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources)
        {
            mError = "";
            mHasFailed = false;
            FontRes *oldRes = aRes;
            aRes = (FontRes*)mFontMap[oldRes->mId];
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        }
        else            
        {
            delete aRes;
            return false;
        }
    }


    XMLParamMap::iterator anItr;
    anItr = theElement.mAttributes.find(_S("image"));
    if (anItr != theElement.mAttributes.end())
        aRes->mImagePath = SexyStringToStringFast(anItr->second);

    anItr = theElement.mAttributes.find(_S("tags"));
    if (anItr != theElement.mAttributes.end())
        aRes->mTags = SexyStringToStringFast(anItr->second);

    if (strncmp(aRes->mPath.c_str(),"!sys:",5)==0)
    {
        aRes->mSysFont = true;
        aRes->mPath = aRes->mPath.substr(5);

        anItr = theElement.mAttributes.find(_S("size"));
        if (anItr==theElement.mAttributes.end())
            return Fail("SysFont needs point size");

        aRes->mSize = sexyatoi(anItr->second.c_str());
        if (aRes->mSize<=0)
            return Fail("SysFont needs point size");
            
        aRes->mBold = theElement.hasAttribute(_S("bold"));
        aRes->mItalic = theElement.hasAttribute(_S("italic"));
        aRes->mShadow = theElement.hasAttribute(_S("shadow"));
        aRes->mUnderline = theElement.hasAttribute(_S("underline"));
    }
    else
        aRes->mSysFont = false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSetDefaults(XMLElement &theElement)
{
    XMLParamMap::iterator anItr;
    anItr = theElement.mAttributes.find(_S("path"));
    if (anItr != theElement.mAttributes.end())
          mDefaultPath = RemoveTrailingSlash(SexyStringToStringFast(anItr->second)) + '/';

    anItr = theElement.mAttributes.find(_S("idprefix"));
    if (anItr != theElement.mAttributes.end())
        mDefaultIdPrefix = RemoveTrailingSlash(SexyStringToStringFast(anItr->second));  

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResources()
{
    for (;;)
    {
        XMLElement aXMLElement;
        if (!mXMLParser->NextElement(&aXMLElement))
            return false;
        
        if (aXMLElement.mType == XMLElement::TYPE_START)
        {
            if (aXMLElement.mValue == _S("Image"))
            {
                if (!ParseImageResource(aXMLElement))
                    return false;

                if (!mXMLParser->NextElement(&aXMLElement))
                    return false;

                if (aXMLElement.mType != XMLElement::TYPE_END)
                    return Fail("Unexpected element found.");
            }
            else if (aXMLElement.mValue == _S("Sound"))
            {
                if (!ParseSoundResource(aXMLElement))
                    return false;

                if (!mXMLParser->NextElement(&aXMLElement))
                    return false;

                if (aXMLElement.mType != XMLElement::TYPE_END)
                    return Fail("Unexpected element found.");
            }
            else if (aXMLElement.mValue == _S("Font"))
            {
                if (!ParseFontResource(aXMLElement))
                    return false;

                if (!mXMLParser->NextElement(&aXMLElement))
                    return false;

                if (aXMLElement.mType != XMLElement::TYPE_END)
                    return Fail("Unexpected element found.");
            }
            else if (aXMLElement.mValue == _S("SetDefaults"))
            {
                if (!ParseSetDefaults(aXMLElement))
                    return false;

                if (!mXMLParser->NextElement(&aXMLElement))
                    return false;

                if (aXMLElement.mType != XMLElement::TYPE_END)
                    return Fail("Unexpected element found.");       
            }
            else
            {
                Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                return false;
            }
        }
        else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
        {
            Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
            return false;
        }       
        else if (aXMLElement.mType == XMLElement::TYPE_END)
        {
            return true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoParseResources()
{
    if (!mXMLParser->HasFailed())
    {
        for (;;)
        {
            XMLElement aXMLElement;
            if (!mXMLParser->NextElement(&aXMLElement))
                break;

            if (aXMLElement.mType == XMLElement::TYPE_START)
            {
                if (aXMLElement.mValue == _S("Resources"))
                {
                    mCurResGroup = SexyStringToStringFast(aXMLElement.mAttributes[_S("id")]);
                    mCurResGroupList = &mResGroupMap[mCurResGroup];

                    if (mCurResGroup.empty())
                    {
                        Fail("No id specified.");
                        break;
                    }

                    if (!ParseResources())
                        break;
                }
                else 
                {
                    Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                    break;
                }
            }
            else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT)
            {
                Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                break;
            }
        }
    }

    if (mXMLParser->HasFailed())
        Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

    delete mXMLParser;
    mXMLParser = NULL;

    return !mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResourcesFile(const std::string& theFilename)
{
    mXMLParser = new XMLParser();
    // TODO. We shouldn't prefix with ResourceFolder here.
    // Better to leave that to XMLParser.
    std::string fname = gSexyAppBase->GetAppResourceFileName(theFilename);
    if (!mXMLParser->OpenFile(fname))
            Fail("Resource file not found: " + fname);


    XMLElement aXMLElement;
    while (!mXMLParser->HasFailed())
    {
        if (!mXMLParser->NextElement(&aXMLElement))
            Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

        if (aXMLElement.mType == XMLElement::TYPE_START)
        {
            if (aXMLElement.mValue != _S("ResourceManifest"))
                break;
            else
                return DoParseResources();
        }
    }
        
    Fail("Expecting ResourceManifest tag");

    return DoParseResources();  
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReparseResourcesFile(const std::string& theFilename)
{
    bool oldDefined = mAllowAlreadyDefinedResources;
    mAllowAlreadyDefinedResources = true;

    bool aResult = ParseResourcesFile(theFilename);

    mAllowAlreadyDefinedResources = oldDefined;

    return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadAlphaGridImage(ImageRes *theRes, DDImage *theImage)
{
#ifdef DEBUG
    Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadAlphaGridImage: '%s'", theRes->mAlphaGridImage.c_str()));
#endif
    ImageLib::Image* anAlphaImage = ImageLib::GetImage(theRes->mAlphaGridImage,true);   
    if (anAlphaImage==NULL)
        return Fail(StrFormat("Failed to load image: %s",theRes->mAlphaGridImage.c_str()));

    std::auto_ptr<ImageLib::Image> aDelAlphaImage(anAlphaImage);

    int aNumRows = theRes->mRows;
    int aNumCols = theRes->mCols;

    int aCelWidth = theImage->GetWidth()/aNumCols;
    int aCelHeight = theImage->GetHeight()/aNumRows;


    if (anAlphaImage->GetWidth()!=aCelWidth || anAlphaImage->GetHeight()!=aCelHeight)
        return Fail(StrFormat("GridAlphaImage size mismatch between %s and %s",theRes->mPath.c_str(),theRes->mAlphaGridImage.c_str()));

    uint32_t *aMasterRowPtr = theImage->mBits;
    for (int i=0; i < aNumRows; i++)
    {
        uint32_t *aMasterColPtr = aMasterRowPtr;
        for (int j=0; j < aNumCols; j++)
        {
            uint32_t* aRowPtr = aMasterColPtr;
            uint32_t* anAlphaBits = anAlphaImage->mBits;
            for (int y=0; y<aCelHeight; y++)
            {
                uint32_t *aDestPtr = aRowPtr;
                for (int x=0; x<aCelWidth; x++)
                {
                    *aDestPtr = (*aDestPtr & 0x00FFFFFF) | ((*anAlphaBits & 0xFF) << 24);
                    ++anAlphaBits;
                    ++aDestPtr;
                }
                aRowPtr += theImage->GetWidth();
            }

            aMasterColPtr += aCelWidth;
        }
        aMasterRowPtr += aCelHeight*theImage->GetWidth();
    }

    theImage->BitsChanged();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadAlphaImage(ImageRes *theRes, DDImage *theImage)
{
#ifdef DEBUG
    Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadAlphaImage: '%s'", theRes->mAlphaGridImage.c_str()));
#endif
#if 0
    SEXY_PERF_BEGIN("ImageLib::GetImage");
#endif
    ImageLib::Image* anAlphaImage = ImageLib::GetImage(theRes->mAlphaImage, true);
#if 0
    SEXY_PERF_END("ImageLib::GetImage");
#endif
    if (anAlphaImage==NULL)
        return Fail(StrFormat("Failed to load image: %s",theRes->mAlphaImage.c_str()));

    std::auto_ptr<ImageLib::Image> aDelAlphaImage(anAlphaImage);

    if (anAlphaImage->GetWidth()!=theImage->GetWidth() || anAlphaImage->GetHeight()!=theImage->GetHeight())
        return Fail(StrFormat("AlphaImage size mismatch between %s and %s",theRes->mPath.c_str(),theRes->mAlphaImage.c_str()));

    uint32_t* aBits1 = theImage->mBits;
    uint32_t* aBits2 = anAlphaImage->mBits;
    int aSize = theImage->GetWidth()*theImage->GetHeight();

    for (int i = 0; i < aSize; i++)
    {
        *aBits1 = (*aBits1 & 0x00FFFFFF) | ((*aBits2 & 0xFF) << 24);
        ++aBits1;
        ++aBits2;
    }

    theImage->BitsChanged();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadImage(ImageRes *theRes)
{
    //bool lookForAlpha = theRes->mAlphaImage.empty() && theRes->mAlphaGridImage.empty() && theRes->mAutoFindAlpha;

#if 0
    SEXY_PERF_BEGIN("ImageLib:GetImage");
#endif
    //ImageLib::Image *anImage = ImageLib::GetImage(theRes->mPath, lookForAlpha);
#if 0
    SEXY_PERF_END("ImageLib:GetImage");
#endif

    bool isNew;
    ImageLib::gAlphaComposeColor = theRes->mAlphaColor;
    SharedImageRef aSharedImageRef = mApp->GetSharedImage(theRes->mPath, &isNew);
    ImageLib::gAlphaComposeColor = 0xFFFFFF;

    DDImage* aDDImage = (DDImage*) aSharedImageRef;
    
    if (aDDImage == NULL)
        return Fail(StrFormat("Failed to load image: %s",theRes->mPath.c_str()));

    if (isNew)
    {
        if (!theRes->mAlphaImage.empty())
        {
            if (!LoadAlphaImage(theRes, aSharedImageRef))
                return false;
        }
        
        if (!theRes->mAlphaGridImage.empty())
        {
            if (!LoadAlphaGridImage(theRes, aSharedImageRef))
                return false;
        }
    }
    
    aDDImage->CommitBits();
    theRes->mImage = aSharedImageRef;
    aDDImage->mPurgeBits = theRes->mPurgeBits;

    if (theRes->mDDSurface)
    {
#if 0
        SEXY_PERF_BEGIN("ResourceManager:DDSurface");
#endif
        aDDImage->CommitBits();
                
        if (!aDDImage->mHasAlpha)
        {
            aDDImage->mWantDDSurface = true;
            aDDImage->mPurgeBits = true;            
        }
#if 0
        SEXY_PERF_END("ResourceManager:DDSurface");
#endif
    }   

    if (theRes->mPalletize)
    {
#if 0
        SEXY_PERF_BEGIN("ResourceManager:Palletize");
#endif
        if (aDDImage->mSurface==NULL) {
            bool done = aDDImage->Palletize();
            Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::DoLoadImage Palletize '%s' %d", theRes->mPath.c_str(), done));
        }
        else
            aDDImage->mWantPal = true;
#if 0
        SEXY_PERF_END("ResourceManager:Palletize");
#endif
    }

#if 0
    if (theRes->mA4R4G4B4)
        aDDImage->mD3DFlags |= D3DImageFlag_UseA4R4G4B4;

    if (theRes->mA8R8G8B8)
        aDDImage->mD3DFlags |= D3DImageFlag_UseA8R8G8B8;

    if (theRes->mMinimizeSubdivisions)
        aDDImage->mD3DFlags |= D3DImageFlag_MinimizeNumSubdivisions;
#endif
    if (theRes->mAnimInfo.mAnimType != AnimType_None)
        aDDImage->mAnimInfo = new AnimInfo(theRes->mAnimInfo);

    aDDImage->SetNumRowsCols(theRes->mRows, theRes->mCols);

    if (aDDImage->mPurgeBits)
        aDDImage->PurgeBits();

    ResourceLoadedHook(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteImage(const std::string &theName)
{
    ReplaceImage(theName,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::LoadImage(const std::string &theName)
{
    ResMap::iterator anItr = mImageMap.find(theName);
    if (anItr == mImageMap.end())
        return NULL;

    ImageRes *aRes = (ImageRes*)anItr->second;
    if ((DDImage*) aRes->mImage != NULL)
        return aRes->mImage;

    if (aRes->mFromProgram)
        return NULL;

    if (!DoLoadImage(aRes))
        return NULL;

    return aRes->mImage;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadSound(SoundRes* theRes)
{
    if (!mApp->mSoundManager)
        return true;                    // Still return true to satisfy ResourceManager loader

    SoundRes *aRes = theRes;

#if 0
    SEXY_PERF_BEGIN("ResourceManager:LoadSound");
#endif
    int aSoundId = mApp->mSoundManager->GetFreeSoundId();
    if (aSoundId<0)
        return Fail("Out of free sound ids");

    if(!mApp->mSoundManager->LoadSound(aSoundId, aRes->mPath))
        return Fail(StrFormat("Failed to load sound: %s",aRes->mPath.c_str()));
#if 0
    SEXY_PERF_END("ResourceManager:LoadSound");
#endif

    if (aRes->mVolume >= 0)
        mApp->mSoundManager->SetBaseVolume(aSoundId, aRes->mVolume);

    if (aRes->mPanning != 0)
        mApp->mSoundManager->SetBasePan(aSoundId, aRes->mPanning);

    aRes->mSoundId = aSoundId;

    ResourceLoadedHook(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadFont(FontRes* theRes)
{
#ifdef DEBUG
    Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::DoLoadFont: '%s'", theRes->mPath.c_str()));
#endif
    Font *aFont = NULL;

#if 0
    SEXY_PERF_BEGIN("ResourceManager:DoLoadFont");
#endif

    if (theRes->mSysFont)
    {
#if 0
        bool bold = theRes->mBold, simulateBold = false;
        if (Sexy::CheckFor98Mill())
        {
            simulateBold = bold;
            bold = false;

        }
        aFont = new SysFont(theRes->mPath,theRes->mSize,bold,theRes->mItalic,theRes->mUnderline);
        SysFont* aSysFont = (SysFont*)aFont;
        aSysFont->mDrawShadow = theRes->mShadow;
        aSysFont->mSimulateBold = simulateBold;
#endif
    }
    else if (theRes->mImagePath.empty())    
    {
        if (strncmp(theRes->mPath.c_str(),"!ref:",5)==0)
        {
            std::string aRefName = theRes->mPath.substr(5);
            Font *aRefFont = GetFont(aRefName);
            if (aRefFont==NULL)
                return Fail("Ref font not found: " + aRefName);

            aFont = aRefFont->Duplicate();
        }
        else
            aFont = new ImageFont(mApp, theRes->mPath);
    }
    else
    {
        Image *anImage = mApp->GetImage(theRes->mImagePath);
        if (anImage==NULL)
            return Fail(StrFormat("Failed to load image: %s",theRes->mImagePath.c_str()));

        theRes->mImage = anImage;
        aFont = new ImageFont(anImage, theRes->mPath);
    }

    ImageFont *anImageFont = dynamic_cast<ImageFont*>(aFont);
    if (anImageFont!=NULL)
    {
        if (anImageFont->mFontData==NULL || !anImageFont->mFontData->mInitialized)
        {
            delete aFont;
            return Fail(StrFormat("Failed to load font: %s",theRes->mPath.c_str()));
        }

        if (!theRes->mTags.empty())
        {
            char aBuf[1024];
            strcpy(aBuf,theRes->mTags.c_str());
            const char *aPtr = strtok(aBuf,", \r\n\t");
            while (aPtr != NULL)
            {
                anImageFont->AddTag(aPtr);
                aPtr = strtok(NULL,", \r\n\t");
            }
            anImageFont->Prepare();
        }
    }

    theRes->mFont = aFont;
#if 0
    SEXY_PERF_END("ResourceManager:DoLoadFont");
#endif

    ResourceLoadedHook(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::LoadFont(const std::string &theName)
{
    ResMap::iterator anItr = mFontMap.find(theName);
    if (anItr == mFontMap.end())
        return NULL;

    FontRes *aRes = (FontRes*)anItr->second;
    if (aRes->mFont != NULL)
        return aRes->mFont;

    if (aRes->mFromProgram)
        return NULL;

    if (!DoLoadFont(aRes))
        return NULL;

    return aRes->mFont;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteFont(const std::string &theName)
{
    ReplaceFont(theName,NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ResourceManager::LoadNextResource()
{
    if (HadError())
        return false;

    if (mCurResGroupList == NULL)
        return false;

    bool retval = false;
    bool done_one = false;
#ifdef DEBUG
    static Timer * timer = new Timer();
    timer->start();
    double start_time = timer->getElapsedTimeInSec();
    Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadNextResource"));
#endif
    while (!done_one && mCurResGroupListItr != mCurResGroupList->end()) {
        BaseRes *aRes = *mCurResGroupListItr++;
        if (aRes->mFromProgram)
            continue;

        switch (aRes->mType) {
        case ResType_Image:
        {
#ifdef DEBUG
            Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadNextResource Image: %s", aRes->mPath.c_str()));
#endif
            ImageRes *anImageRes = (ImageRes*) aRes;
            if ((DDImage*) anImageRes->mImage != NULL)
                continue;

            done_one = true;
            retval = DoLoadImage(anImageRes);
            break;
        }

        case ResType_Sound:
        {
#ifdef DEBUG
            Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadNextResource Sound: %s", aRes->mPath.c_str()));
#endif
            SoundRes *aSoundRes = (SoundRes*) aRes;
            if (aSoundRes->mSoundId != -1)
                continue;

            done_one = true;
            retval = DoLoadSound(aSoundRes);
            break;
        }

        case ResType_Font:
        {
#ifdef DEBUG
            Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadNextResource Font: %s", aRes->mPath.c_str()));
#endif
            FontRes *aFontRes = (FontRes*) aRes;
            if (aFontRes->mFont != NULL)
                continue;

            done_one = true;
            retval = DoLoadFont(aFontRes);
            break;
        }

        default:
            break;
        }
    }

#ifdef DEBUG
    timer->stop();
    Logger::tlog(mLogFacil, 1, Logger::format("ResourceManager::LoadNextResource - done in %8.3f", timer->getElapsedTimeInSec() - start_time));
#endif
    return retval;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::ResourceLoadedHook(BaseRes *theRes)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::StartLoadResources(const std::string &theGroup)
{
    mError = "";
    mHasFailed = false;

    mCurResGroup = theGroup;
    mCurResGroupList = &mResGroupMap[theGroup];
    mCurResGroupListItr = mCurResGroupList->begin();
}

void ResourceManager::StartLoadResourcesThreaded(const std::string &theGroup)
{
    if (mLoadingResourcesStarted)
        return;

    mError = "";
    mHasFailed = false;

    mCurResGroup = theGroup;
    mCurResGroupList = &mResGroupMap[theGroup];
    mCurResGroupListItr = mCurResGroupList->begin();
    
    mLoadingResourcesStarted = true;
    mLoadingResourcesCompleted = false;
    std::string* group = new std::string();
    *group = theGroup; 
    
    struct ThreadData* tdata = new struct ThreadData;
    tdata->manager = this;
    tdata->group = theGroup;

    SDL_CreateThread(&LoadingResourcesStub, tdata);
}

int ResourceManager::LoadingResourcesStub(void *theArg)
{
    struct ThreadData* tdata = (struct ThreadData*)theArg;

    while (tdata->manager->LoadNextResource())
        ;

    if (!tdata->manager->HadError())
    {
        tdata->manager->InsertGroup(tdata->group);
        tdata->manager->SetLoadingResourcesCompleted(true);
        tdata->manager->SetLoadingResourcesStarted(false);
    }
    else {
        tdata->manager->SetLoadingResourcesCompleted(false);
        tdata->manager->SetLoadingResourcesStarted(true);
    }

    delete tdata;
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void ResourceManager::DumpCurResGroup(std::string& theDestStr)
{
    const ResList* rl = &mResGroupMap.find(mCurResGroup)->second;
    ResList::const_iterator it = rl->begin();
    theDestStr = StrFormat("About to dump %d elements from current res group name %s\n", rl->size(), mCurResGroup.c_str());
    
    ResList::const_iterator rl_end = rl->end();
    while (it != rl_end)
    {
        BaseRes* br = *it++;
        std::string prefix = StrFormat("%s: %s\n", br->mId.c_str(), br->mPath.c_str());
        theDestStr += prefix;
        if (br->mFromProgram)
            theDestStr += std::string("     res is from program\n");
        else if (br->mType == ResType_Image)
            theDestStr += std::string("     res is an image\n");
        else if (br->mType == ResType_Sound)
            theDestStr += std::string("     res is a sound\n");
        else if (br->mType == ResType_Font)
            theDestStr += std::string("     res is a font\n");

        if (it == mCurResGroupListItr)
            theDestStr += std::string("iterator has reached mCurResGroupItr\n");

    }

    theDestStr += std::string("Done dumping resources\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadResources(const std::string &theGroup)
{
    mError = "";
    mHasFailed = false;
    StartLoadResources(theGroup);
    while (LoadNextResource())
    {
    }

    if (!HadError())
    {
        mLoadedGroups.insert(theGroup);
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumResources(const std::string &theGroup, ResMap &theMap)
{
    if (theGroup.empty())
        return theMap.size();

    int aCount = 0;
    for (ResMap::iterator anItr = theMap.begin(); anItr != theMap.end(); ++anItr)
    {
        BaseRes *aRes = anItr->second;
        if (aRes->mResGroup==theGroup && !aRes->mFromProgram)
            ++aCount;
    }

    return aCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumImages(const std::string &theGroup)
{
    return GetNumResources(theGroup, mImageMap);
}
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumSounds(const std::string &theGroup)
{
    return GetNumResources(theGroup,mSoundMap);
}
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumFonts(const std::string &theGroup)
{
    return GetNumResources(theGroup, mFontMap);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumResources(const std::string &theGroup)
{
    return GetNumImages(theGroup) + GetNumSounds(theGroup) + GetNumFonts(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::GetImage(const std::string &theId)
{
    ResMap::iterator anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end())
        return ((ImageRes*)anItr->second)->mImage;
    else
        return NULL;
}
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetSound(const std::string &theId)
{
    ResMap::iterator anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end())
        return ((SoundRes*)anItr->second)->mSoundId;
    else
        return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::GetFont(const std::string &theId)
{
    ResMap::iterator anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end())
        return ((FontRes*)anItr->second)->mFont;
    else
        return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SharedImageRef ResourceManager::GetImageThrow(const std::string &theId)
{
    ResMap::iterator anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end())
    {
        ImageRes *aRes = (ImageRes*)anItr->second;
        if ((MemoryImage*) aRes->mImage != NULL)
            return aRes->mImage;

        if (mAllowMissingProgramResources && aRes->mFromProgram)
            return NULL;
    }


    Fail(StrFormat("Image resource not found: %s",theId.c_str()));
    throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetSoundThrow(const std::string &theId)
{
    if (mApp && mApp->mNoSoundNeeded) {
        return -1;
    }
    ResMap::iterator anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end())
    {
        SoundRes *aRes = (SoundRes*)anItr->second;
        if (aRes->mSoundId!=-1)
            return aRes->mSoundId;

        if (mAllowMissingProgramResources && aRes->mFromProgram)
            return -1;
    }


    Fail(StrFormat("Sound resource not found: %s",theId.c_str()));
    throw ResourceManagerException(GetErrorText());     
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Font* ResourceManager::GetFontThrow(const std::string &theId)
{
    ResMap::iterator anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end())
    {
        FontRes *aRes = (FontRes*)anItr->second;
        if (aRes->mFont!=NULL)
            return aRes->mFont;

        if (mAllowMissingProgramResources && aRes->mFromProgram)
            return NULL;
    }

    Fail(StrFormat("Font resource not found: %s",theId.c_str()));
    throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SetAllowMissingProgramImages(bool allow)
{
    mAllowMissingProgramResources = allow;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceImage(const std::string &theId, Image *theImage)
{
    ResMap::iterator anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end())
    {
        anItr->second->DeleteResource();
        ((ImageRes*)anItr->second)->mImage = dynamic_cast<MemoryImage*>(theImage);
        ((ImageRes*)anItr->second)->mImage.mOwnsUnshared = true;
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceSound(const std::string &theId, int theSound)
{
    ResMap::iterator anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end())
    {
        anItr->second->DeleteResource();
        ((SoundRes*)anItr->second)->mSoundId = theSound;
        return true;
    }
    else
        return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceFont(const std::string &theId, Font *theFont)
{
    ResMap::iterator anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end())
    {
        anItr->second->DeleteResource();
        ((FontRes*)anItr->second)->mFont = theFont;
        return true;
    }
    else
        return false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const XMLParamMap& ResourceManager::GetImageAttributes(const std::string &theId)
{
    static XMLParamMap aStrMap;

    ResMap::iterator anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end())
        return anItr->second->mXMLAttributes;
    else
        return aStrMap;
}

