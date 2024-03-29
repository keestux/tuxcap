#ifndef __SEXY_RESOURCEMANAGER_H__
#define __SEXY_RESOURCEMANAGER_H__

#include "Common.h"
#include "Image.h"
#include "Logging.h"
#include "SexyAppBase.h"
#include <string>
#include <map>

namespace ImageLib
{
class Image;
};

namespace Sexy
{

class XMLParser;
class XMLElement;
class Image;
class SoundInstance;
class SexyAppBase;
class Font;

typedef std::map<std::string, std::string>  StringToStringMap;
typedef std::map<SexyString, SexyString>    XMLParamMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ResourceManager
{
protected:
    enum ResType
    {
        ResType_Image,
        ResType_Sound,
        ResType_Font
    };


    struct BaseRes
    {
        ResType mType;
        std::string mId;
        std::string mResGroup;
        std::string mPath;
        XMLParamMap mXMLAttributes;
        bool mFromProgram;
        ResourceManager * mParent;          // For debugging

        BaseRes(ResType t, ResourceManager * resman) : mType(t), mParent(resman) {}
        virtual ~BaseRes() {}
        virtual void DeleteResource() { }
    };

    struct ImageRes : public BaseRes
    {
        Image * mImage;
        std::string mAlphaImage;
        std::string mAlphaGridImage;
        bool mHasAlpha;
        bool mNoAlpha;
        bool mPalletize;
        bool mA4R4G4B4;
        bool mA8R8G8B8;
        bool mDDSurface;
        bool mPurgeBits;
        bool mMinimizeSubdivisions;
        int mRows;
        int mCols;
        uint32_t mAlphaColor;
        AnimInfo mAnimInfo;

        ImageRes(ResourceManager * resman) : BaseRes(ResType_Image, resman) { mImage = NULL; }
        virtual void DeleteResource();
    };

    struct SoundRes : public BaseRes
    {
        int mSoundId;
        double mVolume;
        int mPanning;

        SoundRes(ResourceManager * resman) : BaseRes(ResType_Sound, resman) {}
        virtual void DeleteResource();
    };

    struct FontRes : public BaseRes
    {
        Font *mFont;
        Image *mImage;
        std::string mImagePath;
        std::string mTags;

        // For SysFonts
        bool mSysFont;
        bool mBold;
        bool mItalic;
        bool mUnderline;
        bool mShadow;
        int mSize;


        FontRes(ResourceManager * resman) : BaseRes(ResType_Font, resman) { mFont = NULL; mImage = NULL; }
        virtual void DeleteResource();
    };

    typedef std::multimap<std::string,BaseRes*> ResMap;
    typedef std::list<BaseRes*> ResList;
    typedef std::map<std::string,ResList,StringLessNoCase> ResGroupMap;

    std::set<std::string,StringLessNoCase> mLoadedGroups;

    ResMap                  mImageMap;
    ResMap                  mSoundMap;
    ResMap                  mFontMap;

    XMLParser*              mXMLParser;
    std::string             mError;
    bool                    mHasFailed;
    SexyAppBase*            mApp;
    std::string             mCurResGroup;
    std::string             mDefaultPath;
    std::string             mDefaultIdPrefix;
    bool                    mAllowMissingProgramResources;
    ResGroupMap             mResGroupMap;

    ResList*                mCurResGroupList;
    ResList::iterator       mCurResGroupListItr;

    LoggerFacil *           mLogFacil;

    bool                    Fail(const std::string& theErrorText);
    bool                    Fail(XMLParser * parser, const std::string& theErrorText);

    virtual bool            ParseCommonResource(XMLElement &theElement, BaseRes *theRes, ResMap &theMap);
    virtual bool            ParseSoundResource(XMLElement &theElement);
    virtual bool            ParseImageResource(XMLElement &theElement);
    virtual bool            ParseFontResource(XMLElement &theElement);
    virtual bool            ParseSetDefaults(XMLElement &theElement);
    virtual bool            ParseResources(XMLParser* parser);

    void                    DeleteMap(ResMap &theMap);
    virtual void            DeleteResources(ResMap &theMap, const std::string &theGroup);

    bool                    LoadAlphaGridImage(ImageRes *theRes, DDImage *theImage);
    bool                    LoadAlphaImage(ImageRes *theRes, DDImage *theImage);
    virtual bool            DoLoadImage(ImageRes *theRes);
    virtual bool            DoLoadFont(FontRes* theRes);
    virtual bool            DoLoadSound(SoundRes* theRes);

    int                     GetNumResources(const std::string &theGroup, ResMap &theMap);

    static int              LoadingResourcesStub(void *theArg);
    bool                    mLoadingResourcesStarted;
    bool                    mLoadingResourcesCompleted;
    void                    (*mThreadCompleteCallBack)(void *);
    void                    *mThreadCompleteCallBackArg;

public:
    ResourceManager(SexyAppBase *theApp);
    virtual ~ResourceManager();

    bool                    ParseResourcesFile(const std::string& theFilename);
    bool                    ReparseResourcesFile(const std::string& theFilename);
    bool                    DoParseResources(XMLParser* parser);

    std::string             GetErrorText();
    bool                    HadError();
    bool                    IsGroupLoaded(const std::string &theGroup);

    int                     GetNumImages(const std::string &theGroup);
    int                     GetNumSounds(const std::string &theGroup);
    int                     GetNumFonts(const std::string &theGroup);
    int                     GetNumResources(const std::string &theGroup);

    void                    InsertGroup(const std::string& theGroup) { mLoadedGroups.insert(theGroup); }

    bool                    HasResourceLoadingStarted() { return mLoadingResourcesStarted; }
    bool                    HasResourceLoadingCompleted() { return mLoadingResourcesCompleted; }

    void                    SetLoadingResourcesStarted(bool s) { mLoadingResourcesStarted = s; }
    void                    SetLoadingResourcesCompleted(bool s) { mLoadingResourcesCompleted = s; }

    virtual bool            LoadNextResource();
    virtual void            ResourceLoadedHook(BaseRes *theRes);

    virtual void            StartLoadResources(const std::string &theGroup);
    virtual void            StartLoadResourcesThreaded(const std::string &theGroup);
    virtual void            InstallThreadCompleteCallBack(void (*func)(void *), void * data) { mThreadCompleteCallBack = func; mThreadCompleteCallBackArg = data; }
    virtual bool            LoadResources(const std::string &theGroup);

    bool                    ReplaceImage(const std::string &theId, Image *theImage);
    bool                    ReplaceSound(const std::string &theId, int theSound);
    bool                    ReplaceFont(const std::string &theId, Font *theFont);

    void                    DeleteImage(const std::string &theName);
    Image *                 LoadImage(const std::string &theName);

    void                    DeleteFont(const std::string &theName);
    Font*                   LoadFont(const std::string &theName);

    Image *                 GetImage(const std::string &theId);
    int                     GetSound(const std::string &theId);
    Font*                   GetFont(const std::string &theId);

    // Returns all the XML attributes associated with the image
    const XMLParamMap&      GetImageAttributes(const std::string &theId);

    // These throw a ResourceManagerException if the resource is not found
    virtual Image *         GetImageThrow(const std::string &theId);
    virtual int             GetSoundThrow(const std::string &theId);
    virtual Font*           GetFontThrow(const std::string &theId);

    void                    SetAllowMissingProgramImages(bool allow);

    virtual void            DeleteResources(const std::string &theGroup);
    void                    DeleteExtraImageBuffers(const std::string &theGroup);

    const ResList*          GetCurResGroupList()    {return mCurResGroupList;}
    std::string             GetCurResGroup()        {return mCurResGroup;}
    void                    DumpCurResGroup(std::string& theDestStr);
    bool                    HasEntryResMap(const ResMap& resmap, const std::string& id, const std::string& group);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_EXCEPTION
struct ResourceManagerException : public std::exception
{
    std::string what;
    ResourceManagerException(const std::string &theWhat) : what(theWhat) { }
    ~ResourceManagerException() throw () {} ;
};
#endif

}

#endif // __SEXY_RESOURCEMANAGER_H__
