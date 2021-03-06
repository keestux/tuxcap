#ifndef __PAKINTERFACE_H__
#define __PAKINTERFACE_H__

#include <map>
#include <list>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include "Logging.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class PakCollection;

#ifdef WIN32
typedef FILETIME PakFileTime;
typedef HANDLE PakHandle;
typedef LPCTSTR PakFileNamePtr;
typedef LPWIN32_FIND_DATA PakFindDataPtr;
#else
typedef uint64_t PakFileTime;
typedef void * PakHandle;
typedef const char * PakFileNamePtr;
#define FILE_ATTRIBUTE_DIRECTORY 0x01
typedef struct {
    unsigned dwFileAttributes;
    unsigned long nFileSizeLow;
    PakFileTime ftCreationTime;
    PakFileTime ftLastWriteTime;
    PakFileTime ftLastAccessTime;
    char cFileName[1024];
} PakFindData, * PakFindDataPtr;
#endif

class PakRecord
{
public:
    PakCollection*          mCollection;
    std::string             mFileName;
    PakFileTime             mFileTime;
//private: well, it should be
    int                     mStartPos;
    int                     mSize;
};

typedef std::map<std::string, PakRecord> PakRecordMap;

class PakCollection
{
public:
    PakHandle               mFileHandle;
    PakHandle               mMappingHandle;
    void*                   mDataPtr;
};

typedef std::list<PakCollection> PakCollectionList;

struct PFILE
{
    const PakRecord*        mRecord;
    int                     mPos;
    FILE*                   mFP;
};

// Internal to PakInterface;
class PFindData;

class PakInterfaceBase
{
public:
    PakInterfaceBase();
    virtual ~PakInterfaceBase() {}
public:
    virtual bool            AddPakFile(const std::string& theFileName) = 0;
    virtual bool            isLoaded() const { return false; }

    virtual PFILE*          FOpen(const char* theFileName, const char* theAccess) = 0;
    virtual PFILE*          FOpen(const wchar_t* theFileName, const wchar_t* theAccess) { return NULL; }
    virtual int             FClose(PFILE* theFile) = 0;
    virtual int             FSeek(PFILE* theFile, long theOffset, int theOrigin) = 0;
    virtual int             FTell(PFILE* theFile) = 0;
    virtual int             FSize(PFILE* theFile) = 0;
    virtual size_t          FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) = 0;
    virtual int             FGetC(PFILE* theFile) = 0;
    virtual int             UnGetC(int theChar, PFILE* theFile) = 0;
    virtual char*           FGetS(char* thePtr, int theSize, PFILE* theFile) = 0;
    virtual wchar_t*        FGetS(wchar_t* thePtr, int theSize, PFILE* theFile) { return thePtr; }
    virtual int             FEof(PFILE* theFile) = 0;

    virtual PakHandle       FindFirstFile(PakFileNamePtr lpFileName, PakFindDataPtr lpFindFileData) = 0;
    virtual bool            FindNextFile(PakHandle hFindFile, PakFindDataPtr lpFindFileData) = 0;
    virtual bool            FindClose(PakHandle hFindFile) = 0;

protected:
    virtual const PakRecord* FindPakRecord(const std::string & fname) const = 0;

    std::string     mDir;

    LoggerFacil *   mLogFacil;
};

// TODO. Make this a static member of PakInterfaceBase
extern PakInterfaceBase* GetPakPtr();

class PakInterface : public PakInterfaceBase
{
public:
    PakInterface();
    virtual ~PakInterface();

    virtual bool            AddPakFile(const std::string& theFileName);
    virtual bool            isLoaded() const;

    PFILE*                  FOpen(const char* theFileName, const char* theAccess);
    int                     FClose(PFILE* theFile);
    int                     FSeek(PFILE* theFile, long theOffset, int theOrigin);
    int                     FTell(PFILE* theFile);
    int                     FSize(PFILE* theFile);
    size_t                  FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile);
    int                     FGetC(PFILE* theFile);
    int                     UnGetC(int theChar, PFILE* theFile);
    char*                   FGetS(char* thePtr, int theSize, PFILE* theFile);
    int                     FEof(PFILE* theFile);

    PakHandle               FindFirstFile(PakFileNamePtr lpFileName, PakFindDataPtr lpFindFileData);
    bool                    FindNextFile(PakHandle hFindFile, PakFindDataPtr lpFindFileData);
    bool                    FindClose(PakHandle hFindFile);

protected:
    virtual PFILE*          LoadPakFile(const std::string& theFileName);
    bool                    PFindNext(PFindData* theFindData, PakFindDataPtr lpFindFileData);
    const PakRecord*        FindPakRecord(const std::string & fname) const;

    PakCollectionList       mPakCollectionList;
    PakRecordMap            mPakRecordMap;
};

static inline char * p_wcstombs(const wchar_t * theString)
{
    char * aString;
    size_t length = wcstombs( NULL, theString, 0 );

    aString = new char[length + 1];
    wcstombs(aString, theString, length + 1);

    return aString;
}

static inline PFILE* p_fopen(const char* theFileName, const char* theAccess)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FOpen(theFileName, theAccess);

    // Fallback to do regular IO
    FILE* aFP = fopen(theFileName, theAccess);
    if (aFP == NULL)
        return NULL;
    PFILE* aPFile = new PFILE();
    aPFile->mRecord = NULL;
    aPFile->mPos = 0;
    aPFile->mFP = aFP;
    return aPFile;
}

static inline PFILE* p_fopen(const wchar_t* theFileName, const wchar_t* theAccess)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FOpen(theFileName, theAccess);

    // Fallback to do regular IO
#ifdef WIN32
    FILE* aFP = _wfopen(theFileName, theAccess);
#else
    FILE* aFP;
    char * aFileName = p_wcstombs( theFileName );
    char * aAccess = p_wcstombs( theAccess );

    aFP = fopen(aFileName, aAccess);
    delete [] aFileName;
    delete [] aAccess;
#endif
    if (aFP == NULL)
        return NULL;
    PFILE* aPFile = new PFILE();
    aPFile->mRecord = NULL;
    aPFile->mPos = 0;
    aPFile->mFP = aFP;
    return aPFile;
}

static inline int p_fclose(PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FClose(theFile);

    // Fallback to do regular IO
    int aResult = fclose(theFile->mFP);
    delete theFile;
    return aResult;
}

static inline int p_fseek(PFILE* theFile, long theOffset, int theOrigin)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FSeek(theFile, theOffset, theOrigin);
    return fseek(theFile->mFP, theOffset, theOrigin);
}

static inline int p_ftell(PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FTell(theFile);
    // Fallback to do regular IO
    return ftell(theFile->mFP);
}

static inline int p_size(PFILE* theFile)
{
    int size;
    if (GetPakPtr() != NULL) {
        GetPakPtr()->FSeek(theFile, 0L, SEEK_END);
        size = GetPakPtr()->FTell(theFile);
        GetPakPtr()->FSeek(theFile, 0L, SEEK_SET);
        return size;
    }
    // Fallback to do regular IO
    fseek(theFile->mFP, 0L, SEEK_END);
    size = ftell(theFile->mFP);
    fseek(theFile->mFP, 0L, SEEK_SET);
    return size;
}

static inline size_t p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FRead(thePtr, theSize, theCount, theFile);
    // Fallback to do regular IO
    return fread(thePtr, theSize, theCount, theFile->mFP);
}

static inline size_t p_fwrite(const void* thePtr, int theSize, int theCount, PFILE* theFile)
{
    if (theFile->mFP == NULL)
        return 0;
    // Fallback to do regular IO
    return fwrite(thePtr, theSize, theCount, theFile->mFP);
}

static inline int p_fgetc(PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FGetC(theFile);
    // Fallback to do regular IO
    return fgetc(theFile->mFP);
}

static inline int p_ungetc(int theChar, PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->UnGetC(theChar, theFile);
    // Fallback to do regular IO
    return ungetc(theChar, theFile->mFP);
}

static inline char* p_fgets(char* thePtr, int theSize, PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FGetS(thePtr, theSize, theFile);
    // Fallback to do regular IO
    return fgets(thePtr, theSize, theFile->mFP);
}

static inline wchar_t* p_fgets(wchar_t* thePtr, int theSize, PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FGetS(thePtr, theSize, theFile);
    // Fallback to do regular IO
    return fgetws(thePtr, theSize, theFile->mFP);
}

static inline int p_feof(PFILE* theFile)
{
    if (GetPakPtr() != NULL)
        return GetPakPtr()->FEof(theFile);
    // Fallback to do regular IO
    return feof(theFile->mFP);
}

#endif //__PAKINTERFACE_H__
