#include "KeyCodes.h"

using namespace Sexy;

#define MAX_KEYNAME_LEN 12

typedef struct
{
    char mKeyName[MAX_KEYNAME_LEN];
    KeyCode mKeyCode;
} KeyNameEntry;

KeyNameEntry aKeyCodeArray[] =
{
    {"UNKNOWN", SDLK_UNKNOWN},
    {"TAB", SDLK_TAB},
    {"SPACE", SDLK_SPACE},
    {"BACKSPACE", SDLK_BACKSPACE},
    {"INSERT", SDLK_INSERT},
    {"DELETE", SDLK_DELETE},
    {"RETURN", SDLK_RETURN},
    {"PAUSE", SDLK_PAUSE},
    {"LEFT", SDLK_LEFT},
    {"UP", SDLK_UP},
    {"RIGHT", SDLK_RIGHT},
    {"DOWN", SDLK_DOWN},
    {"ESCAPE", SDLK_ESCAPE},
    {"LSHIFT", SDLK_LSHIFT},
    {"RSHIFT", SDLK_RSHIFT},
    {"LCONTROL", SDLK_LCTRL},
    {"RCONTROL", SDLK_RCTRL},
    {"LALT", SDLK_LALT},
    {"RALT", SDLK_RALT},

    {"F1", SDLK_F1},
    {"F2", SDLK_F2},
    {"F3", SDLK_F3},
    {"F4", SDLK_F4},
    {"F5", SDLK_F5},
    {"F6", SDLK_F6},
    {"F7", SDLK_F7},
    {"F8", SDLK_F8},
    {"F9", SDLK_F9},
    {"F10", SDLK_F10},
    {"F11", SDLK_F11},
    {"F12", SDLK_F12},
    {"F13", SDLK_F13},
    {"F14", SDLK_F14},
    {"F15", SDLK_F15}

#if 0

    {"LBUTTON", SDLK_LBUTTON},  
    {"RBUTTON", SDLK_RBUTTON},
    {"CANCEL", SDLK_CANCEL},
    {"MBUTTON", SDLK_MBUTTON},
    {"BACK", SDLK_BACK},
    {"CLEAR", SDLK_CLEAR},



    {"MENU", SDLK_MENU},

    {"CAPITAL", SDLK_CAPITAL},
    {"KANA", SDLK_KANA},
    {"HANGEUL", SDLK_HANGEUL},
    {"HANGUL", SDLK_HANGUL},
    {"JUNJA", SDLK_JUNJA},
    {"FINAL", SDLK_FINAL},
    {"HANJA", SDLK_HANJA},
    {"KANJI", SDLK_KANJI},

    {"CONVERT", SDLK_CONVERT},
    {"NONCONVERT", SDLK_NONCONVERT},
    {"ACCEPT", SDLK_ACCEPT},
    {"MODECHANGE", SDLK_MODECHANGE},

    {"PRIOR", SDLK_PRIOR},
    {"NEXT", SDLK_NEXT},
    {"END", SDLK_END},
    {"HOME", SDLK_HOME},

    {"SELECT", SDLK_SELECT},
    {"PRINT", SDLK_PRINT},
    {"EXECUTE", SDLK_EXECUTE},
    {"SNAPSHOT", SDLK_SNAPSHOT},
    {"HELP", SDLK_HELP},
    {"LWIN", SDLK_LWIN},
    {"RWIN", SDLK_RWIN},
    {"APPS", SDLK_APPS},
    {"NUMPAD0", SDLK_NUMPAD0},
    {"NUMPAD1", SDLK_NUMPAD1},
    {"NUMPAD2", SDLK_NUMPAD2},
    {"NUMPAD3", SDLK_NUMPAD3},
    {"NUMPAD4", SDLK_NUMPAD4},
    {"NUMPAD5", SDLK_NUMPAD5},
    {"NUMPAD6", SDLK_NUMPAD6},
    {"NUMPAD7", SDLK_NUMPAD7},
    {"NUMPAD8", SDLK_NUMPAD8},
    {"NUMPAD9", SDLK_NUMPAD9},
    {"MULTIPLY", SDLK_MULTIPLY},
    {"ADD", SDLK_ADD},
    {"SEPARATOR", SDLK_SEPARATOR},
    {"SUBTRACT", SDLK_SUBTRACT},
    {"DECIMAL", SDLK_DECIMAL},
    {"DIVIDE", SDLK_DIVIDE},
    {"F16", SDLK_F16},
    {"F17", SDLK_F17},
    {"F18", SDLK_F18},
    {"F19", SDLK_F19},
    {"F20", SDLK_F20},
    {"F21", SDLK_F21},
    {"F22", SDLK_F22},
    {"F23", SDLK_F23},
    {"F24", SDLK_F24},
    {"NUMLOCK", SDLK_NUMLOCK},
    {"SCROLL", SDLK_SCROLL} 
#endif
};

KeyCode Sexy::GetKeyCodeFromName(const std::string& theKeyName)
{
    if (theKeyName.length() >= MAX_KEYNAME_LEN-1)
        return SDLK_UNKNOWN;

        std::string copy = theKeyName;

    if (theKeyName.length() == 1)
    {
          std::transform(copy.begin(), copy.end(), copy.begin(), tolower);

        unsigned char aKeyNameChar = copy[0];

        if (((aKeyNameChar >= 0x5B) && (aKeyNameChar <= 0x7F)) || ((aKeyNameChar >= 0x21) && (aKeyNameChar <= 0x40)))
            return (KeyCode) aKeyNameChar;
    }   

        std::transform(copy.begin(), copy.end(), copy.begin(), toupper);

    for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)
          if (strcmp(copy.c_str(), aKeyCodeArray[i].mKeyName) == 0)
            return aKeyCodeArray[i].mKeyCode;   

    return SDLK_UNKNOWN;
}

const std::string Sexy::GetKeyNameFromCode(const KeyCode& theKeyCode)
{
    if (((theKeyCode >= 0x5B) && (theKeyCode <= 0x7F)) || ((theKeyCode >= 0x21) && (theKeyCode <= 0x40)))
    {
        char aStr[2] = {(char) theKeyCode, 0};
        return aStr;
    }

    for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)
        if (theKeyCode == aKeyCodeArray[i].mKeyCode)
            return aKeyCodeArray[i].mKeyName;   

    return "UNKNOWN";
}

