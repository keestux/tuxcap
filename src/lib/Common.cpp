#include "Common.h"
#include "MTRand.h"
#include "SexyAppBase.h"
#include "PakInterface.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <wctype.h>
#include <dirent.h>
#include <wchar.h>


namespace Sexy
{

bool gDebug = false;
static MTRand gMTRand;

int Rand()
{
    return gMTRand.Next();
}

int Rand(int range)
{
    return gMTRand.Next((uint32_t)range);
}

float Rand(float range)
{
    return gMTRand.Next(range);
}

void SRand(uint32_t theSeed)
{
    gMTRand.SRand(theSeed);
}

std::string GetFileName(const std::string& thePath, bool noExtension)
{
    int aLastSlash = std::max((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

    if (noExtension)
    {
        int aLastDot = (int)thePath.rfind('.');
        if (aLastDot > aLastSlash)
            return thePath.substr(aLastSlash + 1, aLastDot - aLastSlash - 1);
    }

    if (aLastSlash == -1)
        return thePath;
    else
        return thePath.substr(aLastSlash + 1);
}


#if 0
bool CheckFor98Mill()
{
    static bool needOsCheck = true;
    static bool is98Mill = false;

    if (needOsCheck)
    {
        bool invalid = false;
        OSVERSIONINFOEXA osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
        if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
        {
            osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
            if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
                return false;
        }

        needOsCheck = false;
        is98Mill = osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS; // let's check Win95, 98, *AND* ME.
    }

    return is98Mill;
}

bool CheckForVista()
{
    static bool needOsCheck = true;
    static bool isVista = false;

    if (needOsCheck)
    {
        bool invalid = false;
        OSVERSIONINFOEXA osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
        if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
        {
            osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
            if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
                return false;
        }

        needOsCheck = false;
        isVista = osvi.dwMajorVersion >= 6;
    }

    return isVista;
}

std::string URLEncode(const std::string& theString)
{
    char* aHexChars = "0123456789ABCDEF";

    std::string aString;

    for (unsigned i = 0; i < theString.length(); i++)
    {
        switch (theString[i])
        {
        case ' ':
            aString.insert(aString.end(), '+');
            break;
        case '?':
        case '&':
        case '%':
        case '+':
        case '\r':
        case '\n':
        case '\t':
            aString.insert(aString.end(), '%');
            aString.insert(aString.end(), aHexChars[(theString[i] >> 4) & 0xF]);
            aString.insert(aString.end(), aHexChars[(theString[i]     ) & 0xF]);
            break;
        default:
            aString.insert(aString.end(), theString[i]);
        }
    }

    return aString;
}

#endif

// FIXME. There is already a Upper, inlineUpper. Why do we need another?
std::string StringToUpper(const std::string& theString)
{
    std::string aString;

    for (unsigned i = 0; i < theString.length(); i++)
        aString += toupper(theString[i]);

    return aString;
}
std::wstring StringToUpper(const std::wstring& theString)
{
    std::wstring aString;

    for (unsigned i = 0; i < theString.length(); i++)
        aString += towupper(theString[i]);

    return aString;
}

// FIXME. There is already a Lower, inlineLower. Why do we need another?
std::string StringToLower(const std::string& theString)
{
    std::string aString;

    for (unsigned i = 0; i < theString.length(); i++)
        aString += tolower(theString[i]);

    return aString;
}

std::wstring StringToLower(const std::wstring& theString)
{
    std::wstring aString;

    for (unsigned i = 0; i < theString.length(); ++i)
        aString += tolower(theString[i]);

    return aString;
}

#ifdef __ANDROID__
size_t android_mbstowcs(wchar_t *dest, const char * in, int maxlen) 
{
    wchar_t *out = dest;
    int size = 0;
    if (in) {
        while (*in && size<maxlen) {
            if (*in < 128)
                *out++ = *in++;
            else
                *out++ = 0xdc00 + *in++;
            size += 1;     
        }
    }  
    *out = 0;
    return size;
}  

size_t android_wcstombs(char * dest, const wchar_t *source, int maxlen)
{
    wchar_t c;
    int i;
    for (i = 0; i < maxlen && source[i]; i++) {
        c = source[i];
        if (c >= 0xdc80 && c <= 0xdcff) {
            /* UTF-8b surrogate */
            c -= 0xdc00;
        }
        if (dest)
            dest[i] = c;  
    }
    if (dest)
        dest[i] = '\0';
    return i;  
}
#endif

std::wstring StringToWString(const std::string &theString)
{
    std::wstring aString;
    aString.reserve(theString.length());
    for(size_t i = 0; i < theString.length(); ++i)
        aString += (unsigned char)theString[i];
    return aString;
}

std::string WStringToString(const std::wstring &theString)
{
    size_t aRequiredLength = wcstombs( NULL, theString.c_str(), 0 );
    if (aRequiredLength < 16384)
    {
        char aBuffer[16384];
        wcstombs( aBuffer, theString.c_str(), 16384 );
        return std::string(aBuffer);
    }
    else
    {
        assert(aRequiredLength != (size_t)-1);
        if (aRequiredLength == (size_t)-1) return "";

        char* aBuffer = new char[aRequiredLength+1];
        wcstombs( aBuffer, theString.c_str(), aRequiredLength+1 );
        std::string aStr = aBuffer;
        delete[] aBuffer;
        return aStr;
    }
}

SexyString StringToSexyString(const std::string& theString)
{
#ifdef _USE_WIDE_STRING
    return StringToWString(theString);
#else
    return SexyString(theString);
#endif
}

SexyString WStringToSexyString(const std::wstring &theString)
{
#ifdef _USE_WIDE_STRING
    return SexyString(theString);
#else
    return WStringToString(theString);
#endif
}

std::string SexyStringToString(const SexyString& theString)
{
#ifdef _USE_WIDE_STRING
    return WStringToString(theString);
#else
    return std::string(theString);
#endif
}

std::wstring SexyStringToWString(const SexyString& theString)
{
#ifdef _USE_WIDE_STRING
    return std::wstring(theString);
#else
    return StringToWString(theString);
#endif
}
#if 0
std::string Trim(const std::string& theString)
{
    int aStartPos = 0;
    while ( aStartPos < (int) theString.length() && isspace(theString[aStartPos]) )
        aStartPos++;

    int anEndPos = theString.length() - 1;
    while ( anEndPos >= 0 && isspace(theString[anEndPos]) )
        anEndPos--;

    return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}
std::wstring Trim(const std::wstring& theString)
{
    int aStartPos = 0;
    while ( aStartPos < (int) theString.length() && iswspace(theString[aStartPos]) )
        aStartPos++;

    int anEndPos = theString.length() - 1;
    while ( anEndPos >= 0 && iswspace(theString[anEndPos]) )
        anEndPos--;

    return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}
#endif

bool StringToInt(const std::string& theString, int* theIntVal)
{
    *theIntVal = 0;

    if (theString.length() == 0)
        return false;

    int theRadix = 10;
    bool isNeg = false;

    unsigned i = 0;
    if (theString[i] == '-')
    {
        isNeg = true;
        i++;
    }

    for (; i < theString.length(); i++)
    {
        char aChar = theString[i];

        if ((theRadix == 10) && (aChar >= '0') && (aChar <= '9'))
            *theIntVal = (*theIntVal * 10) + (aChar - '0');
        else if ((theRadix == 0x10) &&
            (((aChar >= '0') && (aChar <= '9')) ||
             ((aChar >= 'A') && (aChar <= 'F')) ||
             ((aChar >= 'a') && (aChar <= 'f'))))
        {
            if (aChar <= '9')
                *theIntVal = (*theIntVal * 0x10) + (aChar - '0');
            else if (aChar <= 'F')
                *theIntVal = (*theIntVal * 0x10) + (aChar - 'A') + 0x0A;
            else
                *theIntVal = (*theIntVal * 0x10) + (aChar - 'a') + 0x0A;
        }
        else if (((aChar == 'x') || (aChar == 'X')) && (i == 1) && (*theIntVal == 0))
        {
            theRadix = 0x10;
        }
        else
        {
            *theIntVal = 0;
            return false;
        }
    }

    if (isNeg)
        *theIntVal = -*theIntVal;

    return true;
}

bool StringToInt(const std::wstring& theString, int* theIntVal)
{
    *theIntVal = 0;

    if (theString.length() == 0)
        return false;

    int theRadix = 10;
    bool isNeg = false;

    unsigned i = 0;
    if (theString[i] == '-')
    {
        isNeg = true;
        i++;
    }

    for (; i < theString.length(); i++)
    {
        wchar_t aChar = theString[i];

        if ((theRadix == 10) && (aChar >= L'0') && (aChar <= L'9'))
            *theIntVal = (*theIntVal * 10) + (aChar - L'0');
        else if ((theRadix == 0x10) &&
            (((aChar >= L'0') && (aChar <= L'9')) ||
             ((aChar >= L'A') && (aChar <= L'F')) ||
             ((aChar >= L'a') && (aChar <= L'f'))))
        {
            if (aChar <= L'9')
                *theIntVal = (*theIntVal * 0x10) + (aChar - L'0');
            else if (aChar <= L'F')
                *theIntVal = (*theIntVal * 0x10) + (aChar - L'A') + 0x0A;
            else
                *theIntVal = (*theIntVal * 0x10) + (aChar - L'a') + 0x0A;
        }
        else if (((aChar == L'x') || (aChar == L'X')) && (i == 1) && (*theIntVal == 0))
        {
            theRadix = 0x10;
        }
        else
        {
            *theIntVal = 0;
            return false;
        }
    }

    if (isNeg)
        *theIntVal = -*theIntVal;

    return true;
}

bool StringToDouble(const std::string& theString, double* theDoubleVal)
{
    *theDoubleVal = 0.0;

    if (theString.length() == 0)
        return false;

    bool isNeg = false;

    unsigned i = 0;
    if (theString[i] == '-')
    {
        isNeg = true;
        i++;
    }

    for (; i < theString.length(); i++)
    {
        char aChar = theString[i];

        if ((aChar >= '0') && (aChar <= '9'))
            *theDoubleVal = (*theDoubleVal * 10) + (aChar - '0');
        else if (aChar == '.')
        {
            i++;
            break;
        }
        else
        {
            *theDoubleVal = 0.0;
            return false;
        }
    }

    double aMult = 0.1;
    for (; i < theString.length(); i++)
    {
        char aChar = theString[i];

        if ((aChar >= '0') && (aChar <= '9'))
        {
            *theDoubleVal += (aChar - '0') * aMult;
            aMult /= 10.0;
        }
        else
        {
            *theDoubleVal = 0.0;
            return false;
        }
    }

    if (isNeg)
        *theDoubleVal = -*theDoubleVal;

    return true;
}

bool StringToDouble(const std::wstring& theString, double* theDoubleVal)
{
    *theDoubleVal = 0.0;

    if (theString.length() == 0)
        return false;

    bool isNeg = false;

    unsigned i = 0;
    if (theString[i] == '-')
    {
        isNeg = true;
        i++;
    }

    for (; i < theString.length(); i++)
    {
        wchar_t aChar = theString[i];

        if ((aChar >= L'0') && (aChar <= L'9'))
            *theDoubleVal = (*theDoubleVal * 10) + (aChar - L'0');
        else if (aChar == L'.')
        {
            i++;
            break;
        }
        else
        {
            *theDoubleVal = 0.0;
            return false;
        }
    }

    double aMult = 0.1;
    for (; i < theString.length(); i++)
    {
        wchar_t aChar = theString[i];

        if ((aChar >= L'0') && (aChar <= L'9'))
        {
            *theDoubleVal += (aChar - L'0') * aMult;
            aMult /= 10.0;
        }
        else
        {
            *theDoubleVal = 0.0;
            return false;
        }
    }

    if (isNeg)
        *theDoubleVal = -*theDoubleVal;

    return true;
}

bool StringEndsWith(const std::string& theString, const std::string & theNeedle)
{
    size_t nLen = theNeedle.size();
    size_t sLen = theString.size();
    if (sLen < nLen) {
        return false;
    }
    return theString.substr(sLen - nLen) == theNeedle ? true : false;
}

// TODO: Use <locale> for localization of number output?
SexyString CommaSeperate(int theValue)
{
    if (theValue == 0)
        return _S("0");

    SexyString aCurString;

    int aPlace = 0;
    int aCurValue = theValue;

    while (aCurValue > 0)
    {
        if ((aPlace != 0) && (aPlace % 3 == 0))
            aCurString = _S(',') + aCurString;
        aCurString = (SexyChar) (_S('0') + (aCurValue % 10)) + aCurString;
        aCurValue /= 10;
        aPlace++;
    }

    return aCurString;
}

std::string GetPathFrom(const std::string& theRelPath, const std::string& theDir)
{
    std::string aDriveString;
    std::string aNewPath = theDir;

    if ((theRelPath.length() >= 2) && (theRelPath[1] == ':'))
        return theRelPath;

    char aSlashChar = '/';

    // TODO. Let's unify all slashes to UNIX slashes. Even Windows can handle it.
    if ((theRelPath.find('\\') != std::string::npos) || (theDir.find('\\') != std::string::npos))
        aSlashChar = '\\';

    if ((aNewPath.length() >= 2) && (aNewPath[1] == ':'))
    {
        aDriveString = aNewPath.substr(0, 2);
        aNewPath.erase(aNewPath.begin(), aNewPath.begin()+2);
    }

    // Append a trailing slash if necessary
    if ((aNewPath.length() > 0) && (aNewPath[aNewPath.length()-1] != '\\') && (aNewPath[aNewPath.length()-1] != '/'))
        aNewPath += aSlashChar;

    std::string aTempRelPath = theRelPath;

    for (;;)
    {
        if (aNewPath.length() == 0)
            break;

        int aFirstSlash = aTempRelPath.find('\\');
        int aFirstForwardSlash = aTempRelPath.find('/');

        if ((aFirstSlash == -1) || ((aFirstForwardSlash != -1) && (aFirstForwardSlash < aFirstSlash)))
            aFirstSlash = aFirstForwardSlash;

        if (aFirstSlash == -1)
            break;

        std::string aChDir = aTempRelPath.substr(0, aFirstSlash);

        aTempRelPath.erase(aTempRelPath.begin(), aTempRelPath.begin() + aFirstSlash + 1);

        if (aChDir.compare("..") == 0)
        {
            int aLastDirStart = aNewPath.length() - 1;
            while ((aLastDirStart > 0) && (aNewPath[aLastDirStart-1] != '\\') && (aNewPath[aLastDirStart-1] != '/'))
                aLastDirStart--;

            std::string aLastDir = aNewPath.substr(aLastDirStart, aNewPath.length() - aLastDirStart - 1);
            if (aLastDir.compare("..") == 0)
            {
                aNewPath += "..";
                aNewPath += aSlashChar;
            }
            else
            {
                aNewPath.erase(aNewPath.begin() + aLastDirStart, aNewPath.end());
            }
        }
        else if (aChDir.compare("") == 0)
        {
            aNewPath = aSlashChar;
            break;
        }
        else if (aChDir.compare(".") != 0)
        {
            aNewPath += aChDir + aSlashChar;
            break;
        }
    }

    aNewPath = aDriveString + aNewPath + aTempRelPath;

    if (aSlashChar == '/')
    {
        for (;;)
        {
            int aSlashPos = aNewPath.find('\\');
            if (aSlashPos == -1)
                break;
            aNewPath[aSlashPos] = '/';
        }
    }
    else
    {
        for (;;)
        {
            int aSlashPos = aNewPath.find('/');
            if (aSlashPos == -1)
                break;
            aNewPath[aSlashPos] = '\\';
        }
    }

    return aNewPath;
}

std::vector<std::string> GetFilesInDir(const std::string& theDir)
{
    std::vector<std::string> names;

    if (GetPakPtr()->isLoaded()) {
        PakFindData FindFileData;
        PakHandle handle = GetPakPtr()->FindFirstFile((theDir + "/*").c_str(), &FindFileData);
        if (handle == NULL) {
            goto out;
        }

        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
            // The PakInterface gives us the full filename as they appaer in the pak file
            // In this case we don't want that, the part upto the / is stripped
            // Add first found file
            names.push_back(GetFileName(FindFileData.cFileName));
        }

        while (GetPakPtr()->FindNextFile(handle, &FindFileData)) {
            if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
                // Add next found file
                names.push_back(GetFileName(FindFileData.cFileName));
            }
        }

        GetPakPtr()->FindClose(handle);
    }
    else {
#ifdef WIN32
        // Use the first-first, find-next method
#else
        DIR* dir = opendir(theDir.c_str());
        if (dir == NULL)
            return names;

        struct dirent* sdir;

        while ((sdir = readdir(dir)) != NULL) {
            std::string s = sdir->d_name;
            names.push_back(s);
        }

        closedir(dir);
#endif
    }
out:
    return names;
}

std::string GetFileDir(const std::string& thePath, bool withSlash)
{
    int aLastSlash = std::max((int) thePath.rfind('\\'), (int) thePath.rfind('/'));

    if (aLastSlash == -1) {
        if (withSlash) {
            return "./";
        } else {
            return ".";
        }
    }
    else {
        if (withSlash)
            return thePath.substr(0, aLastSlash + 1);
        else
            return thePath.substr(0, aLastSlash);
    }
}

bool AllowAllAccess(const std::string& theFileName)
{
    chmod(theFileName.c_str(), 0666);
    return true;
}

bool FileExists(const std::string& theFileName)
{
    struct stat s;
    int res = stat(theFileName.c_str(), &s);

    if (res == -1)
        return false;

    return S_ISREG(s.st_mode);
}

bool CreateFile(const std::string& theFileName)
{
    FILE* f = fopen(theFileName.c_str(), "w");
    if (f != NULL)
        fclose(f);
    else
        return false;
    return true;
}

std::string GetFullPath(const std::string& theRelPath)
{
    return GetPathFrom(theRelPath, GetCurDir());
}

std::string GetCurDir()
{
    char aDir[512];
    return getcwd(aDir, sizeof(aDir));
}

#if 0


bool Deltree(const std::string& thePath)
{
    bool success = true;

    std::string aSourceDir = thePath;

    if (aSourceDir.length() < 2)
        return false;

    if ((aSourceDir[aSourceDir.length() - 1] != '\\') ||
        (aSourceDir[aSourceDir.length() - 1] != '/'))
        aSourceDir += "\\";

    WIN32_FIND_DATAA aFindData;

    HANDLE aFindHandle = FindFirstFileA((aSourceDir + "*.*").c_str(), &aFindData);
    if (aFindHandle == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        if ((aFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if ((strcmp(aFindData.cFileName, ".") != 0) &&
                (strcmp(aFindData.cFileName, "..") != 0))
            {
                // Follow the directory
                if (!Deltree(aSourceDir + aFindData.cFileName))
                    success = false;
            }
        }
        else
        {
            std::string aFullName = aSourceDir + aFindData.cFileName;
            if (!DeleteFileA(aFullName.c_str()))
                success = false;
        }
    }
    while (FindNextFileA(aFindHandle, &aFindData));
    FindClose(aFindHandle);

    if (rmdir(thePath.c_str()) == 0)
        success = false;

    return success;
}

std::string AddTrailingSlash(const std::string& theDirectory, bool backSlash)
{
    if (!theDirectory.empty())
    {
        char aChar = theDirectory[theDirectory.length()-1];
        if (aChar!='\\' && aChar!='/')
            return theDirectory + (backSlash?'\\':'/');
        else
            return theDirectory;
    }
    else
        return "";
}


time_t GetFileDate(const std::string& theFileName)
{
    time_t aFileDate = 0;

    WIN32_FIND_DATAA aFindData;
    HANDLE aFindHandle = ::FindFirstFileA(theFileName.c_str(), &aFindData);

    if (aFindHandle != INVALID_HANDLE_VALUE)
    {
        FILETIME aFileTime = aFindData.ftLastWriteTime;

        //FileTimeToUnixTime(&aFileTime, &aFileDate, FALSE);

        LONGLONG ll = (__int64) aFileTime.dwHighDateTime << 32;
        ll = ll + aFileTime.dwLowDateTime - 116444736000000000;
        aFileDate = (time_t) (ll/10000000);

        FindClose(aFindHandle);
    }

    return aFileDate;
}
#endif
std::string vformat(const char* fmt, va_list argPtr)
{
    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
    char stackBuffer[bufSize];

    int attemptedSize = bufSize - 1;

    int numChars = 0;
#ifdef _WIN32
    numChars = _vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#else
    numChars = vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#endif

    //cout << "NumChars: " << numChars << endl;

    if ((numChars >= 0) && (numChars <= attemptedSize))
    {
        // Needed for case of 160-character printf thing
        stackBuffer[numChars] = '\0';

        // Got it on the first try.
        return std::string(stackBuffer);
    }

    // Now use the heap.
    char* heapBuffer = NULL;

    while (((numChars == -1) || (numChars > attemptedSize)) &&
        (attemptedSize < maxSize))
    {
        // Try a bigger size
        attemptedSize *= 2;
        heapBuffer = (char*)realloc(heapBuffer, (attemptedSize + 1));
#ifdef _WIN32
        numChars = _vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#else
        numChars = vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#endif
    }

    heapBuffer[numChars] = 0;

    std::string result = std::string(heapBuffer);

    free(heapBuffer);
    return result;
}

//overloaded StrFormat: should only be used by the xml strings
std::string StrFormat(const char* fmt ...)
{
    va_list argList;
    va_start(argList, fmt);
    std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}
#if 0
std::wstring vformat(const wchar_t* fmt, va_list argPtr)
{
    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
    wchar_t stackBuffer[bufSize];

    int attemptedSize = bufSize - 1;

    int numChars = 0;
#ifdef _WIN32
    numChars = _vsnwprintf(stackBuffer, attemptedSize, fmt, argPtr);
#else
    numChars = vsnwprintf(stackBuffer, attemptedSize, fmt, argPtr);
#endif

    //cout << "NumChars: " << numChars << endl;

    if ((numChars >= 0) && (numChars <= attemptedSize))
    {
        // Needed for case of 160-character printf thing
        stackBuffer[numChars] = '\0';

        // Got it on the first try.
        return std::wstring(stackBuffer);
    }

    // Now use the heap.
    wchar_t* heapBuffer = NULL;

    while (((numChars == -1) || (numChars > attemptedSize)) &&
        (attemptedSize < maxSize))
    {
        // Try a bigger size
        attemptedSize *= 2;
        //heapBuffer = (wchar_t*)realloc(heapBuffer, (attemptedSize + 1));
                heapBuffer = (wchar_t*)realloc(heapBuffer, (attemptedSize + 1)*sizeof(wchar_t));
#ifdef _WIN32
        numChars = _vsnwprintf(heapBuffer, attemptedSize, fmt, argPtr);
#else
        numChars = vsnwprintf(heapBuffer, attemptedSize, fmt, argPtr);
#endif
    }

    heapBuffer[numChars] = 0;

    std::wstring result = std::wstring(heapBuffer);

    free(heapBuffer);

    return result;
}
//overloaded StrFormat: should only be used by the xml strings
std::wstring StrFormat(const wchar_t* fmt ...)
{
    va_list argList;
    va_start(argList, fmt);
    std::wstring result = vformat(fmt, argList);
    va_end(argList);

    return result;
}
std::string Evaluate(const std::string& theString, const DefinesMap& theDefinesMap)
{
    std::string anEvaluatedString = theString;

    for (;;)
    {
        int aPercentPos = anEvaluatedString.find('%');

        if (aPercentPos == std::string::npos)
            break;

        int aSecondPercentPos = anEvaluatedString.find('%', aPercentPos + 1);
        if (aSecondPercentPos == std::string::npos)
            break;

        std::string aName = anEvaluatedString.substr(aPercentPos + 1, aSecondPercentPos - aPercentPos - 1);

        std::string aValue;
        DefinesMap::const_iterator anItr = theDefinesMap.find(aName);
        if (anItr != theDefinesMap.end())
            aValue = anItr->second;
        else
            aValue = "";

        anEvaluatedString.erase(anEvaluatedString.begin() + aPercentPos, anEvaluatedString.begin() + aSecondPercentPos + 1);
        anEvaluatedString.insert(anEvaluatedString.begin() + aPercentPos, aValue.begin(), aValue.begin() + aValue.length());
    }

    return anEvaluatedString;
}
#endif
std::string XMLDecodeString(const std::string& theString)
{
    std::string aNewString;

    for (size_t i = 0; i < theString.length(); i++)
    {
        char c = theString[i];

        if (c == '&')
        {
            size_t aSemiPos = theString.find(';', i);

            if (aSemiPos != std::string::npos)
            {
                std::string anEntName = theString.substr(i+1, aSemiPos-i-1);
                i = aSemiPos;

                if (anEntName == "lt")
                    c = '<';
                else if (anEntName == "amp")
                    c = '&';
                else if (anEntName == "gt")
                    c = '>';
                else if (anEntName == "quot")
                    c = '"';
                else if (anEntName == "apos")
                    c = '\'';
                else if (anEntName == "nbsp")
                    c = ' ';
                else if (anEntName == "cr")
                    c = '\n';
            }
        }

        aNewString += c;
    }

    return aNewString;
}

std::wstring XMLDecodeString(const std::wstring& theString)
{
    std::wstring aNewString;

    for (size_t i = 0; i < theString.length(); i++)
    {
        wchar_t c = theString[i];

        if (c == L'&')
        {
            size_t aSemiPos = theString.find(L';', i);

            if (aSemiPos != std::wstring::npos)
            {
                std::wstring anEntName = theString.substr(i+1, aSemiPos-i-1);
                i = aSemiPos;

                if (anEntName == L"lt")
                    c = L'<';
                else if (anEntName == L"amp")
                    c = L'&';
                else if (anEntName == L"gt")
                    c = L'>';
                else if (anEntName == L"quot")
                    c = L'"';
                else if (anEntName == L"apos")
                    c = L'\'';
                else if (anEntName == L"nbsp")
                    c = L' ';
                else if (anEntName == L"cr")
                    c = L'\n';
            }
        }

        aNewString += c;
    }

    return aNewString;
}


std::string XMLEncodeString(const std::string& theString)
{
    std::string aNewString;

    bool hasSpace = false;

    for (size_t i = 0; i < theString.length(); i++)
    {
        char c = theString[i];

        if (c == ' ')
        {
            if (hasSpace)
            {
                aNewString += "&nbsp;";
                continue;
            }

            hasSpace = true;
        }
        else
            hasSpace = false;

        /*if ((uchar) c >= 0x80)
        {
            // Convert to UTF
            aNewString += (char) (0xC0 | ((c >> 6) & 0xFF));
            aNewString += (char) (0x80 | (c & 0x3F));
        }
        else*/
        {
            switch (c)
            {
            case '<':
                aNewString += "&lt;";
                break;
            case '&':
                aNewString += "&amp;";
                break;
            case '>':
                aNewString += "&gt;";
                break;
            case '"':
                aNewString += "&quot;";
                break;
            case '\'':
                aNewString += "&apos;";
                break;
            case '\n':
                aNewString += "&cr;";
                break;
            default:
                aNewString += c;
                break;
            }
        }
    }

    return aNewString;
}

std::wstring XMLEncodeString(const std::wstring& theString)
{
    std::wstring aNewString;

    bool hasSpace = false;

    for (size_t i = 0; i < theString.length(); i++)
    {
        wchar_t c = theString[i];

        if (c == ' ')
        {
            if (hasSpace)
            {
                aNewString += L"&nbsp;";
                continue;
            }

            hasSpace = true;
        }
        else
            hasSpace = false;

        /*if ((uchar) c >= 0x80)
        {
            // Convert to UTF
            aNewString += (char) (0xC0 | ((c >> 6) & 0xFF));
            aNewString += (char) (0x80 | (c & 0x3F));
        }
        else*/
        {
            switch (c)
            {
            case L'<':
                aNewString += L"&lt;";
                break;
            case L'&':
                aNewString += L"&amp;";
                break;
            case L'>':
                aNewString += L"&gt;";
                break;
            case L'"':
                aNewString += L"&quot;";
                break;
            case L'\'':
                aNewString += L"&apos;";
                break;
            case L'\n':
                aNewString += L"&cr;";
                break;
            default:
                aNewString += c;
                break;
            }
        }
    }

    return aNewString;
}

// TODO. Eliminate this one because we have inlineUpper too.
std::string Upper(const std::string& _data)
{
    std::string s = _data;
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
}
std::wstring Upper(const std::wstring& _data)
{
    std::wstring s = _data;
    std::transform(s.begin(), s.end(), s.begin(), towupper);
    return s;
}

// TODO. Eliminate this one because we have inlineLower too.
std::string Lower(const std::string& _data)
{
    std::string s = _data;
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

std::wstring Lower(const std::wstring& _data)
{
    std::wstring s = _data;
    std::transform(s.begin(), s.end(), s.begin(), towlower);
    return s;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int StrFindNoCase(const char *theStr, const char *theFind)
{
    int p1, p2;
    int cp = 0;
    const int len1 = (int) strlen(theStr);
    const int len2 = (int) strlen(theFind);
    while (cp < len1) {
        p1 = cp;
        p2 = 0;
        while (p1 < len1 && p2 < len2) {
            if (tolower(theStr[p1]) != tolower(theFind[p2]))
                break;

            p1++;
            p2++;
        }
        if (p2 == len2)
            return p1 - len2;

        cp++;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength)
{
    int i;
    char c1 = 0, c2 = 0;
    for (i=0; i<maxLength; i++)
    {
        c1 = tolower(*theStr++);
        c2 = tolower(*thePrefix++);

        if (c1==0 || c2==0)
            break;

        if (c1!=c2)
            return false;
    }

    return c2==0 || i==maxLength;
}

std::string RemoveTrailingSlash(const std::string& theDirectory)
{
    int aLen = theDirectory.length();

    if ((aLen > 0) && ((theDirectory[aLen-1] == '\\') || (theDirectory[aLen-1] == '/')))
        return theDirectory.substr(0, aLen - 1);
    else
        return theDirectory;
}

// TODO. Rename this function to ReplaceSlash
std::string BuildIniName(std::string copy, const std::string& theSubstitute)
{
    size_t pos = copy.find_first_of("\\/");

    while (pos != std::string::npos) {
        copy.replace(pos, 1, theSubstitute);
        pos = copy.find_first_of("\\/");
    }

    return copy;
}

std::string ReplaceBackSlashes(std::string copy)
{
    size_t pos = copy.find_first_of('\\');

    while (pos != std::string::npos) {
        if (pos + 1 < copy.size() && copy.at(pos + 1) == '\\') {
            copy.replace(pos + 1, 1, "/");
            copy.erase(pos, 1);
        } else {
            copy.replace(pos, 1, "/");
        }
        pos = copy.find_first_of('\\');
    }

    return copy;
}

void MkDir(const std::string& theDir)
{
    std::string aPath = theDir;

    size_t aCurPos = 0;
    for (;;)
    {
        size_t aSlashPos = aPath.find_first_of("\\/", aCurPos);
        if (aSlashPos == std::string::npos)
        {
            mkdir(aPath.c_str(), 0777);
            break;
        }

        aCurPos = aSlashPos+1;

        std::string aCurPath = aPath.substr(0, aSlashPos);
        mkdir(aCurPath.c_str(), 0777);
    }
}

bool IsDir(const std::string & theDir)
{
    struct stat dir_stat;

    if (stat(theDir.c_str(), &dir_stat) == 0) {

        if (S_ISDIR(dir_stat.st_mode)) {
            return true;
        }
    }
    return false;
}

std::string findResourceFolder(const std::string & dir)
{
    // Look for file main.pak
    if (FileExists(dir + "main.pak")) {
        return dir;
    }
    // Look for dir res, Resources
    if (IsDir(dir + "res")) {
        return dir + "res/";
    }
    else if (IsDir(dir + "Resources")) {
        return dir + "Resources/";
    }
    else if (IsDir(dir + "resources")) {
        // Linux is case sensitive. We may want to use lower case directory names
        return dir + "resources/";
    }
    return "";
}

static std::string stripCurrentDir(std::string dir)
{
    while (dir.find("./") == 0) {
        dir = dir.substr(2);
    }
    return dir;
}

std::string determineResourceFolder(std::string bindir)
{
    bindir = stripCurrentDir(bindir);
    std::string rscDir;

    // Look in <bindir>/..
    rscDir = findResourceFolder(bindir + "../");
    if (rscDir != "") {
        return rscDir;
    }
    // Look in <bindir>/../lib
    rscDir = findResourceFolder(bindir + "../lib/");
    if (rscDir != "") {
        return rscDir;
    }
    // Look in <bindir>
    rscDir = findResourceFolder(bindir);
    if (rscDir != "") {
        return rscDir;
    }
    // Elsewhere?
    return "";
}

}
