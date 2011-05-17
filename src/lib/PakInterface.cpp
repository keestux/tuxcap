//
// PakInterface.cpp
// A class to read files from a "pak" file, a minimalistic tar file. See tuxpak how to build one.

// NOMINMAX is to ????
#define NOMINMAX
#include "PakInterface.h"
#include "Common.h"
#include "Logging.h"

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define stricmp(x, y) strcasecmp(x, y)
#define strnicmp(x, y, l) strncasecmp(x, y, l)
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

enum
{
    FILEFLAGS_END = 0x80,           // indicates end of header
};

class PFindData
{
public:
    PakHandle               mWHandle;
    std::string             mFindCriteria;
    std::string             mDir;           // The name used in opendir
    PakRecordMap::const_iterator mLastFind;
};

static PakInterfaceBase* gPakInterfaceP;
PakInterfaceBase* GetPakPtr()
{
    return gPakInterfaceP;
}

PakInterfaceBase::PakInterfaceBase()
{
    mLogFacil = NULL;
#ifdef DEBUG
    mLogFacil = LoggerFacil::find("pakinterface");
    Logger::log(mLogFacil, 3, "new PakInterfaceBase");
#endif

    mDir = "";
}

PakInterface::PakInterface()
{
    if (gPakInterfaceP == NULL)
        gPakInterfaceP = this;
}

PakInterface::~PakInterface()
{
}

bool PakInterface::isLoaded() const
{
    return !mPakCollectionList.empty();
}

bool PakInterface::AddPakFile(const std::string& theFileName)
{
    // We record the directory where the pak file is found.
    // Later, if a file name lookup takes place and if the
    // file name starts with this prefix, we'll strip it.
    std::string myDir = Sexy::GetFileDir(std::string(theFileName), true);
    if (myDir.find("./") == 0) {
        myDir = myDir.substr(2);
    }
    mDir = myDir;
#ifdef DEBUG
    Logger::log(mLogFacil, 1, "AddPakFile, using directory: " + Logger::quote(mDir));
#endif

    PFILE* aFP = LoadPakFile(theFileName);
    if (aFP == NULL) {
        return false;
    }
    // Use the pakcollection created in LoadPakFile
    PakCollection* aPakCollection = &mPakCollectionList.back();

    uint32_t aMagic = 0;
    FRead(&aMagic, sizeof(aMagic), 1, aFP);
#if __BIG_ENDIAN__
    aMagic = Sexy::SwapFourBytes(aMagic);
#endif
    if (aMagic != 0xBAC04AC0) {
        FClose(aFP);
        return false;
    }

    uint32_t aVersion = 0;
    FRead(&aVersion, sizeof(aVersion), 1, aFP);
#if __BIG_ENDIAN__
    aVersion = Sexy::SwapFourBytes(aVersion);
#endif
    if (aVersion != 0) {
        FClose(aFP);
        return false;
    }

    int aPos = 0;
    for (;;)
    {
        uchar aFlags = 0;
        int aCount = FRead(&aFlags, sizeof(aFlags), 1, aFP);
        if ((aFlags & FILEFLAGS_END) || (aCount == 0))
            break;

        uchar aNameWidth = 0;
        char aName[256];
        FRead(&aNameWidth, sizeof(aNameWidth), 1, aFP);
        FRead(aName, sizeof(aName[0]), aNameWidth, aFP);
        aName[aNameWidth] = 0;
        for (int i = 0; i < aNameWidth; i++) {
            if (aName[i] == '\\') {
                aName[i] = '/';         // Normalize to UNIX path separators
            }
        }

        int32_t aSrcSize = 0;
        FRead(&aSrcSize, sizeof(aSrcSize), 1, aFP);
#if __BIG_ENDIAN__
        aSrcSize = Sexy::SwapFourBytes(aSrcSize);
#endif
        PakFileTime aFileTime;
        FRead(&aFileTime, sizeof(aFileTime), 1, aFP);

        PakRecordMap::iterator aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(aName, PakRecord())).first;
        PakRecord* aPakRecord = &(aRecordItr->second);
        aPakRecord->mCollection = aPakCollection;
        aPakRecord->mFileName = aName;
        aPakRecord->mStartPos = aPos;
        aPakRecord->mSize = aSrcSize;
        aPakRecord->mFileTime = aFileTime;

        aPos += aSrcSize;
    }

    // Now fix file starts. The file position is now at the start of the data.
    int dataOffset = FTell(aFP);
    for (PakRecordMap::iterator myItr = mPakRecordMap.begin(); myItr != mPakRecordMap.end(); myItr++) {
        PakRecord* aPakRecord = &myItr->second;
        // Only the records added for this PakCollection
        if (aPakRecord->mCollection == aPakCollection)
            aPakRecord->mStartPos += dataOffset;
    }

    FClose(aFP);
#ifdef DEBUG
    //Logger::log(mLogFacil, 3, "AddPakFile: mPakRecordMap.size()=" + Logger::int2str(mPakRecordMap.size()));
#endif

    return true;
}

// Load PAK file in memory
PFILE* PakInterface::LoadPakFile(const std::string& theFileName)
{
#ifdef WIN32
    HANDLE aFileHandle = CreateFile(theFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (aFileHandle == INVALID_HANDLE_VALUE)
        return false;

    int aFileSize = GetFileSize(aFileHandle, 0);

    HANDLE aFileMapping = CreateFileMapping(aFileHandle, NULL, PAGE_READONLY, 0, aFileSize, NULL);
    if (aFileMapping == NULL)
    {
        CloseHandle(aFileHandle);
        return false;
    }

    void* aPtr = MapViewOfFile(aFileMapping, FILE_MAP_READ, 0, 0, aFileSize);
    if (aPtr == NULL)
    {
        CloseHandle(aFileMapping);
        CloseHandle(aFileHandle);
        return false;
    }

    mPakCollectionList.push_back(PakCollection());
    PakCollection* aPakCollection = &mPakCollectionList.back();

    aPakCollection->mFileHandle = aFileHandle;
    aPakCollection->mMappingHandle = aFileMapping;
    aPakCollection->mDataPtr = aPtr;
#else
    int aFileHandle = open(theFileName.c_str(), O_RDONLY);

    if (aFileHandle < 0) {
#ifdef DEBUG
        Logger::log(mLogFacil, 1, "Pak file not found: " + Logger::quote(theFileName));
#endif
        return false;
    }

    struct stat buf;
    fstat(aFileHandle, &buf);

    size_t aFileSize = buf.st_size;

    void* aFileMapping = mmap(NULL, aFileSize, PROT_READ, MAP_SHARED, (int)aFileHandle, 0);
    if (aFileMapping == MAP_FAILED) {
#ifdef DEBUG
        Logger::log(mLogFacil, 1, "Oops. Failed to mmap");
#endif
        close(aFileHandle);
        return false;
    }

    // Create a new PakCollection
    mPakCollectionList.push_back(PakCollection());
    PakCollection* aPakCollection = &mPakCollectionList.back();

    aPakCollection->mFileHandle = (PakHandle)aFileHandle;
    aPakCollection->mMappingHandle = (PakHandle)aFileMapping;
    aPakCollection->mDataPtr = aFileMapping;
#endif

    PakRecordMap::iterator aRecordItr;

    // Create a new entry for the PAK file itself.
    // This is so that we can do FOpen, FRead using the freshly setup mmap.
    aRecordItr = mPakRecordMap.insert(PakRecordMap::value_type(theFileName, PakRecord())).first;
    PakRecord* aPakRecord = &(aRecordItr->second);

    aPakRecord->mCollection = aPakCollection;
    aPakRecord->mFileName = theFileName;
    aPakRecord->mStartPos = 0;
    aPakRecord->mSize = aFileSize;

    PFILE* aFP = NULL;
    {
        // TODO. Use FindPakRecord()
        PakRecordMap::iterator anItr = mPakRecordMap.find(theFileName);
        if (anItr != mPakRecordMap.end()) {
            aFP = new PFILE;
            aFP->mRecord = &anItr->second;          // This should be the new PakRecord created above.
            aFP->mPos = 0;
            aFP->mFP = NULL;
        }
    }
    return aFP;
}

PFILE* PakInterface::FOpen(const char* theFileName, const char* anAccess)
{
    // Possibly strip "./" at start of filename.
    if (theFileName && theFileName[0] == '.' && theFileName[1] == '/') {
        theFileName += 2;
    }

    // Normalize path using UNIX separators
    std::string tmpName = theFileName;
    int len = strlen(theFileName);
    for (int i = 0; i < len; i++) {
        if (tmpName[i] == '\\') {
            tmpName[i] = '/';
        }
    }

#ifdef DEBUG
    Logger::log(mLogFacil, 3, Logger::format("FOpen: %s, mode: %s\n", tmpName.c_str(), anAccess));
#endif
    if ((stricmp(anAccess, "r") == 0) || (stricmp(anAccess, "rb") == 0) || (stricmp(anAccess, "rt") == 0))
    {
        std::string myTmpName = tmpName;

        // Possibly strip Resources directory prefix
        if (myTmpName.find(mDir) == 0) {
            myTmpName = myTmpName.substr(mDir.length());
        }

        const PakRecord * pr = FindPakRecord(myTmpName.c_str());
        if (pr) {
#ifdef DEBUG
            Logger::log(mLogFacil, 2, Logger::format("FOpen: '%s' found in PAK file\n", myTmpName.c_str()));
#endif
            PFILE* aPFP = new PFILE;
            aPFP->mRecord = pr;
            aPFP->mPos = 0;
            aPFP->mFP = NULL;
            return aPFP;
        }
    }

    FILE* aFP = fopen(tmpName.c_str(), anAccess);
    if (aFP) {
        PFILE* aPFP = new PFILE;
        aPFP->mRecord = NULL;
        aPFP->mPos = 0;
        aPFP->mFP = aFP;

#ifdef DEBUG
        Logger::log(mLogFacil, 2, Logger::format("FOpen: '%s' found on filesystem", tmpName.c_str()));
#endif
        return aPFP;
    }
#ifdef DEBUG
    Logger::log(mLogFacil, 2, Logger::format("FOpen: '%s' not found", tmpName.c_str()));
#endif

    // ???? TODO. Perhaps we should try the actual file name too.

    return NULL;
}

int PakInterface::FClose(PFILE* theFile)
{
    if (theFile->mRecord == NULL)
        fclose(theFile->mFP);
    delete theFile;
    return 0;
}

int PakInterface::FSeek(PFILE* theFile, long theOffset, int theOrigin)
{
    if (theFile->mRecord != NULL)
    {
        if (theOrigin == SEEK_SET)
            theFile->mPos = theOffset;
        else if (theOrigin == SEEK_END)
            theFile->mPos = theFile->mRecord->mSize - theOffset;
        else if (theOrigin == SEEK_CUR)
            theFile->mPos += theOffset;

        theFile->mPos = std::max(std::min(theFile->mPos, theFile->mRecord->mSize), 0);
        return 0;
    }
    else
        return fseek(theFile->mFP, theOffset, theOrigin);
}

int PakInterface::FTell(PFILE* theFile)
{
    if (theFile->mRecord != NULL)
        return theFile->mPos;
    else
        return ftell(theFile->mFP);
}

int PakInterface::FSize(PFILE* theFile)
{
    if (theFile->mRecord != NULL)
        return theFile->mRecord->mSize;
    else {
        // There are two ways to get the file size.
        // 1. Seek to the end, and do a tell (this screws up current position.
        // 2. Use fstat
        fseek(theFile->mFP, 0L, SEEK_END);
        int size = ftell(theFile->mFP);
        fseek(theFile->mFP, 0L, SEEK_SET);
        return size;
    }
}

size_t PakInterface::FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile)
{
    if (theFile->mRecord != NULL)
    {
        int aSizeBytes = std::min(theElemSize*theCount, theFile->mRecord->mSize - theFile->mPos);

        uchar* src = (uchar*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos;
        uchar* dest = (uchar*) thePtr;
        for (int i = 0; i < aSizeBytes; i++)
            *(dest++) = (*src++) ^ 0xF7; // 'Decrypt'
        theFile->mPos += aSizeBytes;
        return aSizeBytes / theElemSize;
    }

    return fread(thePtr, theElemSize, theCount, theFile->mFP);
}

int PakInterface::FGetC(PFILE* theFile)
{
    if (theFile->mRecord != NULL)
    {
        for (;;)
        {
            if (theFile->mPos >= theFile->mRecord->mSize)
                return EOF;
            char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++) ^ 0xF7;
            if (aChar != '\r')
                return (uchar) aChar;
        }
    }

    return fgetc(theFile->mFP);
}

int PakInterface::UnGetC(int theChar, PFILE* theFile)
{
    if (theFile->mRecord != NULL)
    {
        // This won't work if we're not pushing the same chars back in the stream
        theFile->mPos = std::max(theFile->mPos - 1, 0);
        return theChar;
    }

    return ungetc(theChar, theFile->mFP);
}

char* PakInterface::FGetS(char* thePtr, int theSize, PFILE* theFile)
{
    if (theFile->mRecord != NULL)
    {
        int anIdx = 0;
        while (anIdx < theSize)
        {
            if (theFile->mPos >= theFile->mRecord->mSize)
            {
                if (anIdx == 0)
                    return NULL;
                break;
            }
            char aChar = *((char*) theFile->mRecord->mCollection->mDataPtr + theFile->mRecord->mStartPos + theFile->mPos++) ^ 0xF7;
            if (aChar != '\r')
                thePtr[anIdx++] = aChar;
            if (aChar == '\n')
                break;
        }
        thePtr[anIdx] = 0;
        return thePtr;
    }

    return fgets(thePtr, theSize, theFile->mFP);
}

int PakInterface::FEof(PFILE* theFile)
{
    if (theFile->mRecord != NULL)
        return theFile->mPos >= theFile->mRecord->mSize;
    else
        return feof(theFile->mFP);
}

//
bool PakInterface::PFindNext(PFindData* theFindData, PakFindDataPtr lpFindFileData)
{
    // Possibly strip Resources directory prefix
    std::string find_criteria = theFindData->mFindCriteria;
    if (find_criteria.find(mDir) == 0) {
        find_criteria = find_criteria.substr(mDir.length());
    }

    size_t aStarPos = find_criteria.find('*');
    if (aStarPos != std::string::npos) {
        // Use substr until * for the match
        find_criteria = find_criteria.substr(0, aStarPos);
    }
    size_t find_criteria_length = find_criteria.size();

    PakRecordMap::const_iterator anItr;
    if (theFindData->mLastFind == mPakRecordMap.end())
        anItr = mPakRecordMap.begin();
    else {
        // Continue from last time
        anItr = theFindData->mLastFind;
        anItr++;
    }

    while (anItr != mPakRecordMap.end()) {
        std::string aFileName = anItr->first;
        const PakRecord * aPakRecord = &anItr->second;

        if (aFileName != find_criteria) {
            // Not a full match. Try a partial match
            if (aFileName.size() > find_criteria_length && aFileName.substr(0, find_criteria_length) == find_criteria) {
            }
            else {
                ++anItr;
                continue;
            }
        }

        // This is a match
        strncpy(lpFindFileData->cFileName, aFileName.c_str(), 1023);
        lpFindFileData->cFileName[1023] = 0;
        lpFindFileData->nFileSizeLow = aPakRecord->mSize;
        lpFindFileData->ftCreationTime = aPakRecord->mFileTime;
        lpFindFileData->ftLastWriteTime = aPakRecord->mFileTime;
        lpFindFileData->ftLastAccessTime = aPakRecord->mFileTime;
        theFindData->mLastFind = anItr;
        return true;
    }
    return false;
}

#ifndef WIN32
static bool
PakEmuFindNext(PFindData * aFindData, PakFindDataPtr lpFindFileData)
{
    if (aFindData == NULL || aFindData->mWHandle == NULL) {
        return false;
    }
    DIR * dir = (DIR *)aFindData->mWHandle;

    struct dirent entry;
    struct dirent * result;

    while (readdir_r(dir, &entry, &result) == 0 && result) {
        if (!(result->d_type == DT_REG || result->d_type == DT_DIR)) {
            continue;
        }

        std::string fname = aFindData->mDir + result->d_name;
        struct stat buf;
        if (stat (fname.c_str(), &buf) != 0) {
            break;
        }

        memset(lpFindFileData, 0, sizeof(PakFindData));
        // FIXME. dirent only has filename of size 255
        strncpy(lpFindFileData->cFileName, result->d_name, 255);
        lpFindFileData->cFileName[1023] = 0;        // Just to be sure to terminate the string.
        if (S_ISDIR(buf.st_mode))
            lpFindFileData->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        lpFindFileData->nFileSizeLow = buf.st_size;
        lpFindFileData->ftCreationTime = buf.st_mtime;
        lpFindFileData->ftLastWriteTime = buf.st_mtime;
        lpFindFileData->ftLastAccessTime = buf.st_atime;
        return true;
    }

    return false;
}
#endif

PakHandle PakInterface::FindFirstFile(PakFileNamePtr lpFileName, PakFindDataPtr lpFindFileData)
{
    PFindData* aFindData = new PFindData;

    aFindData->mFindCriteria = lpFileName;
    aFindData->mLastFind = mPakRecordMap.end();
    aFindData->mDir = "";
#ifdef WIN32
    aFindData->mWHandle = INVALID_HANDLE_VALUE;

    if (PFindNext(aFindData, lpFindFileData))
        return (HANDLE) aFindData;

    aFindData->mWHandle = ::FindFirstFile(aFindData->mFindCriteria.c_str(), lpFindFileData);
    if (aFindData->mWHandle != INVALID_HANDLE_VALUE)
        return (HANDLE) aFindData;

    delete aFindData;
    return INVALID_HANDLE_VALUE;
#else
    aFindData->mWHandle = NULL;

    if (PFindNext(aFindData, lpFindFileData))
        return (PakHandle) aFindData;

    std::string find_criteria = aFindData->mFindCriteria;
    size_t aStarPos = find_criteria.find('*');
    if (aStarPos != std::string::npos) {
        // Use substr until * for the opendir
        find_criteria = find_criteria.substr(0, aStarPos);
        aFindData->mDir = find_criteria;
    }
    // TODO/FIXME If find_criteria is not a file, then what?
    aFindData->mWHandle = opendir(find_criteria.c_str());
    if (aFindData->mWHandle == NULL)
        goto fail;

    if (PakEmuFindNext(aFindData, lpFindFileData)) {
        // Return the first found file in lpFindFileData
        return (PakHandle) aFindData;
    }

 fail:
    delete aFindData;
    return NULL;
#endif
}

bool PakInterface::FindNextFile(PakHandle hFindFile, PakFindDataPtr lpFindFileData)
{
    PFindData* aFindData = (PFindData*) hFindFile;

#ifdef WIN32
    if (aFindData->mWHandle == INVALID_HANDLE_VALUE)
    {
        if (PFindNext(aFindData, lpFindFileData))
            return true;

        aFindData->mWHandle = ::FindFirstFile(aFindData->mFindCriteria.c_str(), lpFindFileData);
        return (aFindData->mWHandle != INVALID_HANDLE_VALUE);
    }

    return ::FindNextFile(aFindData->mWHandle, lpFindFileData);

#else
    if (aFindData->mWHandle == NULL) {
        if (PFindNext(aFindData, lpFindFileData))
            return true;

        // Nothing found by PFindNext
        std::string find_criteria = aFindData->mFindCriteria;
        size_t aStarPos = find_criteria.find('*');
        if (aStarPos != std::string::npos) {
            // Use substr until * for the opendir
            find_criteria = find_criteria.substr(0, aStarPos);
            aFindData->mDir = find_criteria;
        }
        // TODO/FIXME If find_criteria is not a file, then what?
        aFindData->mWHandle = opendir(aFindData->mFindCriteria.c_str());
    }

    return PakEmuFindNext(aFindData, lpFindFileData);
#endif
}

bool PakInterface::FindClose(PakHandle hFindFile)
{
    PFindData* aFindData = (PFindData*) hFindFile;

#ifdef WIN32
    if (aFindData->mWHandle != INVALID_HANDLE_VALUE)
        ::FindClose(aFindData->mWHandle);
#else
    if (aFindData->mWHandle != NULL)
        closedir((DIR*)aFindData->mWHandle);
#endif

    delete aFindData;
    return true;
}

const PakRecord * PakInterface::FindPakRecord(const std::string & fname) const
{
    PakRecordMap::const_iterator anItr = mPakRecordMap.find(fname);
    if (anItr != mPakRecordMap.end()) {
        return &anItr->second;
    }
    return NULL;
}
