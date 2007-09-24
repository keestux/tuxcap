#ifndef __SEXYAPPFRAMEWORK_COMMON_H__
#define __SEXYAPPFRAMEWORK_COMMON_H__

#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <stdarg.h>
#include <wchar.h>
#include "SDL/SDL.h"
#include "SDL/SDL_keysym.h"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
const Uint32 SDL_rmask = 0x000000FF;
const Uint32 SDL_gmask = 0x0000FF00;
const Uint32 SDL_bmask = 0x00FF0000;
const Uint32 SDL_amask = 0xFF000000;
#else
const Uint32 SDL_rmask = 0xFF000000;
const Uint32 SDL_gmask = 0x00FF0000;
const Uint32 SDL_bmask = 0x0000FF00;
const Uint32 SDL_amask = 0x000000FF;
#endif

//FIXME map popcap keycodes to sdl

#define KeyCode SDLKey

//#define USE_AUDIERE 

#ifdef _USE_WIDE_STRING

#define sexystricmp			wcscasecmp
typedef std::wstring		SexyString;
#define _S(x)				L ##x

#if 0

#include "ModVal.h"

#define sexystrncmp			wcsncmp
#define sexystrcmp			wcscmp

#define sexysscanf			swscanf
#define sexyatoi			_wtoi
#define sexystrcpy			wcscpy

#define SexyStringToStringFast(x)	WStringToString(x)
#define SexyStringToWStringFast(x)	(x)
#define StringToSexyStringFast(x)	StringToWString(x)
#define WStringToSexyStringFast(x)	(x)

#ifndef SEXYFRAMEWORK_NO_REDEFINE_WIN_API
// Redefine the functions and structs we need to be wide-string
#undef CreateWindowEx
#undef RegisterClass
#undef MessageBox
#undef ShellExecute
#undef GetTextExtentPoint32
#undef RegisterWindowMessage
#undef CreateMutex
#undef DrawTextEx
#undef TextOut

#define CreateWindowEx				CreateWindowExW
#define RegisterClass				RegisterClassW
#define WNDCLASS					WNDCLASSW
#define MessageBox					MessageBoxW
#define ShellExecute				ShellExecuteW
#define GetTextExtentPoint32		GetTextExtentPoint32W
#define RegisterWindowMessage		RegisterWindowMessageW
#define CreateMutex					CreateMutexW
#define DrawTextEx					DrawTextExW
#define TextOut						TextOutW
#endif
#endif
#else
typedef std::string			SexyString;
#define _S(x)				x


#define sexystrncmp			strncmp
#define sexystrcmp			strcmp
#define sexystricmp			strcasecmp
#define sexysscanf			sscanf
#define sexyatoi			atoi
#define sexystrcpy			strcpy

#define SexyStringToStringFast(x)	(x)
#define SexyStringToWStringFast(x)	StringToWString(x)
#define StringToSexyStringFast(x)	(x)
#define WStringToSexyStringFast(x)	WStringToString(x)

#endif

#define LONG_BIGE_TO_NATIVE(l) (((l >> 24) & 0xFF) | ((l >> 8) & 0xFF00) | ((l << 8) & 0xFF0000) | ((l << 24) & 0xFF000000))
#define WORD_BIGE_TO_NATIVE(w) (((w >> 8) & 0xFF) | ((w << 8) & 0xFF00))
#define LONG_LITTLEE_TO_NATIVE(l) (l)
#define WORD_LITTLEE_TO_NATIVE(w) (w)

#define LENGTH(anyarray) (sizeof(anyarray) / sizeof(anyarray[0]))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long int64;

typedef std::map<std::string, std::string>		DefinesMap;
typedef std::map<std::wstring, std::wstring>	WStringWStringMap;
typedef SexyString::value_type					SexyChar;
#define HAS_SEXYCHAR

namespace Sexy
{

const ulong SEXY_RAND_MAX = 0x7FFFFFFF;

extern bool			gDebug;

int					Rand();
int					Rand(int range);
float				Rand(float range);
void				SRand(ulong theSeed);
extern std::string	vformat(const char* fmt, va_list argPtr);
extern std::string	StrFormat(const char* fmt ...);
std::string			GetAppDataFolder();
void				SetAppDataFolder(const std::string& thePath);
std::string			GetPathFrom(const std::string& theRelPath, const std::string& theDir);
std::string			GetFileDir(const std::string& thePath, bool withSlash = false);
std::string			GetFileName(const std::string& thePath, bool noExtension = false);
std::string			RemoveTrailingSlash(const std::string& theDirectory);
 std::string BuildIniName(const std::string& theString, const std::string& theSubstitute);
void				MkDir(const std::string& theDir);
#if 0
bool				FileExists(const std::string& theFileName);
bool				Deltree(const std::string& thePath);





std::string			AddTrailingSlash(const std::string& theDirectory, bool backSlash = false);
time_t				GetFileDate(const std::string& theFileName);
std::string			GetCurDir();
std::string			GetFullPath(const std::string& theRelPath);

bool				AllowAllAccess(const std::string& theFileName);

std::string			Trim(const std::string& theString);
std::wstring		Trim(const std::wstring& theString);
extern std::wstring	StrFormat(const wchar_t* fmt ...);
extern std::wstring	vformat(const wchar_t* fmt, va_list argPtr);
#endif
std::string			Upper(const std::string& theData);
std::wstring		Upper(const std::wstring& theData);
std::string			Lower(const std::string& theData);
std::wstring		Lower(const std::wstring& theData);
std::string			StringToUpper(const std::string& theString);
std::wstring		StringToUpper(const std::wstring& theString);
std::string			URLEncode(const std::string& theString);
std::string			StringToLower(const std::string& theString);
std::wstring		StringToLower(const std::wstring& theString);
std::wstring		StringToWString(const std::string &theString);
std::string			WStringToString(const std::wstring &theString);
SexyString			StringToSexyString(const std::string& theString);
SexyString			WStringToSexyString(const std::wstring& theString);
std::string			SexyStringToString(const SexyString& theString);
std::wstring		SexyStringToWString(const SexyString& theString);
bool				StringToInt(const std::string theString, int* theIntVal);
bool				StringToDouble(const std::string theString, double* theDoubleVal);
bool				StringToInt(const std::wstring theString, int* theIntVal);
bool				StringToDouble(const std::wstring theString, double* theDoubleVal);
int					StrFindNoCase(const char *theStr, const char *theFind);
bool				StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength = 10000000);
SexyString			CommaSeperate(int theValue);
std::string			Evaluate(const std::string& theString, const DefinesMap& theDefinesMap);
std::string			XMLDecodeString(const std::string& theString);
std::string			XMLEncodeString(const std::string& theString);
std::wstring		XMLDecodeString(const std::wstring& theString);
std::wstring		XMLEncodeString(const std::wstring& theString);


inline void			inlineUpper(std::string &theData)
{
    //std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int) theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = toupper(theData[i]);
	}
}
#if 0
inline void			inlineUpper(std::wstring &theData)
{
    //std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int) theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = towupper(theData[i]);
	}
}
#endif

inline void			inlineLower(std::string &theData)
{
    std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void			inlineLower(std::wstring &theData)
{
    std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void			inlineLTrim(std::string &theData, const std::string& theChars = " \t\r\n")
{
    theData.erase(0, theData.find_first_not_of(theChars));
}

inline void			inlineLTrim(std::wstring &theData, const std::wstring& theChars = L" \t\r\n")
{
    theData.erase(0, theData.find_first_not_of(theChars));
}


inline void			inlineRTrim(std::string &theData, const std::string& theChars = " \t\r\n")
{
    theData.resize(theData.find_last_not_of(theChars) + 1);
}

inline void			inlineTrim(std::string &theData, const std::string& theChars = " \t\r\n")
{
	inlineRTrim(theData, theChars);
	inlineLTrim(theData, theChars);
}

struct StringLessNoCase { bool operator()(const std::string &s1, const std::string &s2) const { return strcasecmp(s1.c_str(),s2.c_str())<0; } };

}
#endif //__SEXYAPPFRAMEWORK_COMMON_H__
