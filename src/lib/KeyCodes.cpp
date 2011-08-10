#include "KeyCodes.h"
#include <SDL_keycode.h>

using namespace Sexy;

#define MAX_KEYNAME_LEN 12

typedef struct
{
    char mKeyName[MAX_KEYNAME_LEN];
    KeyCode mKeyCode;
} KeyNameEntry;

KeyNameEntry aKeyCodeArray[] =
{
    {"UNKNOWN",    KEYCODE_UNKNOWN},
    {"LBUTTON",    KEYCODE_LBUTTON},
    {"RBUTTON",    KEYCODE_RBUTTON},
    {"CANCEL",     KEYCODE_CANCEL},
    {"MBUTTON",    KEYCODE_MBUTTON},
    {"BACK",       KEYCODE_BACK},
    {"TAB",        KEYCODE_TAB},
    {"CLEAR",      KEYCODE_CLEAR},
    {"RETURN",     KEYCODE_RETURN},
    {"SHIFT",      KEYCODE_SHIFT},
    {"CONTROL",    KEYCODE_CONTROL},
    {"MENU",       KEYCODE_MENU},
    {"PAUSE",      KEYCODE_PAUSE},
    {"CAPITAL",    KEYCODE_CAPITAL},
    {"KANA",       KEYCODE_KANA},
    {"HANGEUL",    KEYCODE_HANGEUL},
    {"HANGUL",     KEYCODE_HANGUL},
    {"JUNJA",      KEYCODE_JUNJA},
    {"FINAL",      KEYCODE_FINAL},
    {"HANJA",      KEYCODE_HANJA},
    {"KANJI",      KEYCODE_KANJI},
    {"ESCAPE",     KEYCODE_ESCAPE},
    {"CONVERT",    KEYCODE_CONVERT},
    {"NONCONVERT", KEYCODE_NONCONVERT},
    {"ACCEPT",     KEYCODE_ACCEPT},
    {"MODECHANGE", KEYCODE_MODECHANGE},
    {"SPACE",      KEYCODE_SPACE},
    {"PRIOR",      KEYCODE_PRIOR},
    {"NEXT",       KEYCODE_NEXT},
    {"END",        KEYCODE_END},
    {"HOME",       KEYCODE_HOME},
    {"LEFT",       KEYCODE_LEFT},
    {"UP",         KEYCODE_UP},
    {"RIGHT",      KEYCODE_RIGHT},
    {"DOWN",       KEYCODE_DOWN},
    {"SELECT",     KEYCODE_SELECT},
    {"PRINT",      KEYCODE_PRINT},
    {"EXECUTE",    KEYCODE_EXECUTE},
    {"SNAPSHOT",   KEYCODE_SNAPSHOT},
    {"INSERT",     KEYCODE_INSERT},
    {"DELETE",     KEYCODE_DELETE},
    {"HELP",       KEYCODE_HELP},
    {"LWIN",       KEYCODE_LWIN},
    {"RWIN",       KEYCODE_RWIN},
    {"APPS",       KEYCODE_APPS},
    {"NUMPAD0",    KEYCODE_NUMPAD0},
    {"NUMPAD1",    KEYCODE_NUMPAD1},
    {"NUMPAD2",    KEYCODE_NUMPAD2},
    {"NUMPAD3",    KEYCODE_NUMPAD3},
    {"NUMPAD4",    KEYCODE_NUMPAD4},
    {"NUMPAD5",    KEYCODE_NUMPAD5},
    {"NUMPAD6",    KEYCODE_NUMPAD6},
    {"NUMPAD7",    KEYCODE_NUMPAD7},
    {"NUMPAD8",    KEYCODE_NUMPAD8},
    {"NUMPAD9",    KEYCODE_NUMPAD9},
    {"MULTIPLY",   KEYCODE_MULTIPLY},
    {"ADD",        KEYCODE_ADD},
    {"SEPARATOR",  KEYCODE_SEPARATOR},
    {"SUBTRACT",   KEYCODE_SUBTRACT},
    {"DECIMAL",    KEYCODE_DECIMAL},
    {"DIVIDE",     KEYCODE_DIVIDE},
    {"F1",         KEYCODE_F1},
    {"F2",         KEYCODE_F2},
    {"F3",         KEYCODE_F3},
    {"F4",         KEYCODE_F4},
    {"F5",         KEYCODE_F5},
    {"F6",         KEYCODE_F6},
    {"F7",         KEYCODE_F7},
    {"F8",         KEYCODE_F8},
    {"F9",         KEYCODE_F9},
    {"F10",        KEYCODE_F10},
    {"F11",        KEYCODE_F11},
    {"F12",        KEYCODE_F12},
    {"F13",        KEYCODE_F13},
    {"F14",        KEYCODE_F14},
    {"F15",        KEYCODE_F15},
    {"F16",        KEYCODE_F16},
    {"F17",        KEYCODE_F17},
    {"F18",        KEYCODE_F18},
    {"F19",        KEYCODE_F19},
    {"F20",        KEYCODE_F20},
    {"F21",        KEYCODE_F21},
    {"F22",        KEYCODE_F22},
    {"F23",        KEYCODE_F23},
    {"F24",        KEYCODE_F24},
    {"NUMLOCK",    KEYCODE_NUMLOCK},
    {"SCROLL",     KEYCODE_SCROLL},
#if 0
    {"BACKSPACE",  KEYCODE_BACKSPACE},
    {"LSHIFT",     KEYCODE_LSHIFT},
    {"RSHIFT",     KEYCODE_RSHIFT},
    {"LCONTROL",   KEYCODE_LCTRL},
    {"RCONTROL",   KEYCODE_RCTRL},
    {"LALT",       KEYCODE_LALT},
    {"RALT",       KEYCODE_RALT},
#endif
};

KeyCode Sexy::GetKeyCodeFromName(const std::string& theKeyName)
{
    if (theKeyName.length() >= MAX_KEYNAME_LEN-1)
        return KEYCODE_UNKNOWN;

    std::string copy = theKeyName;

    if (theKeyName.length() == 1)
    {
        std::transform(copy.begin(), copy.end(), copy.begin(), tolower);

        int aKeyNameChar = copy[0];

        // 5A => 'Z'
        // TODO. Check caller if 0x21..0x41 is acceptable
        if (((aKeyNameChar >= 0x5B) && (aKeyNameChar <= 0x7F)) || ((aKeyNameChar >= 0x21) && (aKeyNameChar <= 0x40)))
            return (KeyCode) aKeyNameChar;
    }

    // Convert to upper and lookup the name in the array
    std::transform(copy.begin(), copy.end(), copy.begin(), toupper);
    for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)
        if (strcmp(copy.c_str(), aKeyCodeArray[i].mKeyName) == 0)
            return aKeyCodeArray[i].mKeyCode;

    return KEYCODE_UNKNOWN;
}

const std::string Sexy::GetKeyNameFromCode(const KeyCode& theKeyCode)
{
    if ((theKeyCode >= KEYCODE_ASCIIBEGIN) && (theKeyCode <= KEYCODE_ASCIIEND))
    {
        char aStr[2] = {(char) theKeyCode, 0};
        return aStr;
    }

    // Lookup the code
    for (size_t i = 0; i < sizeof(aKeyCodeArray)/sizeof(aKeyCodeArray[0]); i++)
        if (theKeyCode == aKeyCodeArray[i].mKeyCode)
            return aKeyCodeArray[i].mKeyName;

    return "UNKNOWN";
}

KeyCode Sexy::GetKeyCodeFromSDLKey(SDLKey key)
{
    switch (key)
    {
    case SDLK_UNKNOWN:     return KEYCODE_UNKNOWN;
    //case SDLK_LBUTTON:     return KEYCODE_LBUTTON;
    //case SDLK_RBUTTON:     return KEYCODE_RBUTTON;
    //case SDLK_CANCEL:      return KEYCODE_CANCEL;
    //case SDLK_MBUTTON:     return KEYCODE_MBUTTON;
    case SDLK_BACKSPACE:   return KEYCODE_BACK;
    case SDLK_TAB:         return KEYCODE_TAB;
    case SDLK_CLEAR:       return KEYCODE_CLEAR;
    case SDLK_RETURN:      return KEYCODE_RETURN;
    case SDLK_LSHIFT:      return KEYCODE_SHIFT;
    case SDLK_RSHIFT:      return KEYCODE_SHIFT;
    case SDLK_LCTRL:       return KEYCODE_CONTROL;
    case SDLK_RCTRL:       return KEYCODE_CONTROL;
    case SDLK_MENU:        return KEYCODE_MENU;
    case SDLK_PAUSE:       return KEYCODE_PAUSE;
#if 0
    case SDLK_CAPITAL:     return KEYCODE_CAPITAL;
    case SDLK_KANA:        return KEYCODE_KANA;
    case SDLK_HANGEUL:     return KEYCODE_HANGEUL;
    case SDLK_HANGUL:      return KEYCODE_HANGUL;
    case SDLK_JUNJA:       return KEYCODE_JUNJA;
    case SDLK_FINAL:       return KEYCODE_FINAL;
    case SDLK_HANJA:       return KEYCODE_HANJA;
    case SDLK_KANJI:       return KEYCODE_KANJI;
#endif
    case SDLK_ESCAPE:      return KEYCODE_ESCAPE;
    //case SDLK_CONVERT:     return KEYCODE_CONVERT;
    //case SDLK_NONCONVERT:  return KEYCODE_NONCONVERT;
    //case SDLK_ACCEPT:      return KEYCODE_ACCEPT;
    //case SDLK_MODECHANGE:  return KEYCODE_MODECHANGE;
    case SDLK_SPACE:       return KEYCODE_SPACE;
    //case SDLK_PRIOR:       return KEYCODE_PRIOR;
    //case SDLK_NEXT:        return KEYCODE_NEXT;
    case SDLK_END:         return KEYCODE_END;
    case SDLK_HOME:        return KEYCODE_HOME;
    case SDLK_LEFT:        return KEYCODE_LEFT;
    case SDLK_UP:          return KEYCODE_UP;
    case SDLK_RIGHT:       return KEYCODE_RIGHT;
    case SDLK_DOWN:        return KEYCODE_DOWN;
    //case SDLK_SELECT:      return KEYCODE_SELECT;
    case SDLK_PRINT:       return KEYCODE_PRINT;
    //case SDLK_EXECUTE:     return KEYCODE_EXECUTE;
    //case SDLK_SNAPSHOT:    return KEYCODE_SNAPSHOT;
    case SDLK_INSERT:      return KEYCODE_INSERT;
    case SDLK_DELETE:      return KEYCODE_DELETE;
    case SDLK_HELP:        return KEYCODE_HELP;
    case SDLK_0:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 0);
    case SDLK_1:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 1);
    case SDLK_2:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 2);
    case SDLK_3:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 3);
    case SDLK_4:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 4);
    case SDLK_5:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 5);
    case SDLK_6:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 6);
    case SDLK_7:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 7);
    case SDLK_8:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 8);
    case SDLK_9:           return (KeyCode)(KEYCODE_ASCIIBEGIN + 9);
    //case SDLK_ASCIIBEGIN:  return KEYCODE_ASCIIBEGIN;
    //case SDLK_ASCIIEND:    return KEYCODE_ASCIIEND;
    //case SDLK_LWIN:        return KEYCODE_LWIN;
    //case SDLK_RWIN:        return KEYCODE_RWIN;
    //case SDLK_APPS:        return KEYCODE_APPS;
    case SDLK_KP0:         return KEYCODE_NUMPAD0;
    case SDLK_KP1:         return KEYCODE_NUMPAD1;
    case SDLK_KP2:         return KEYCODE_NUMPAD2;
    case SDLK_KP3:         return KEYCODE_NUMPAD3;
    case SDLK_KP4:         return KEYCODE_NUMPAD4;
    case SDLK_KP5:         return KEYCODE_NUMPAD5;
    case SDLK_KP6:         return KEYCODE_NUMPAD6;
    case SDLK_KP7:         return KEYCODE_NUMPAD7;
    case SDLK_KP8:         return KEYCODE_NUMPAD8;
    case SDLK_KP9:         return KEYCODE_NUMPAD9;
#if SDL_VERSION_ATLEAST(1,3,0)
    case SDLK_KP_MULTIPLY:    return KEYCODE_MULTIPLY;
    case SDLK_KP_PLUS:        return KEYCODE_ADD;
    //case SDLK_KP_SEPARATOR:   return KEYCODE_SEPARATOR;
    case SDLK_KP_MINUS:    return KEYCODE_SUBTRACT;
    case SDLK_KP_DECIMAL:     return KEYCODE_DECIMAL;
    case SDLK_KP_DIVIDE:      return KEYCODE_DIVIDE;
#endif
    case SDLK_F1:          return KEYCODE_F1;
    case SDLK_F2:          return KEYCODE_F2;
    case SDLK_F3:          return KEYCODE_F3;
    case SDLK_F4:          return KEYCODE_F4;
    case SDLK_F5:          return KEYCODE_F5;
    case SDLK_F6:          return KEYCODE_F6;
    case SDLK_F7:          return KEYCODE_F7;
    case SDLK_F8:          return KEYCODE_F8;
    case SDLK_F9:          return KEYCODE_F9;
    case SDLK_F10:         return KEYCODE_F10;
    case SDLK_F11:         return KEYCODE_F11;
    case SDLK_F12:         return KEYCODE_F12;
    case SDLK_F13:         return KEYCODE_F13;
    case SDLK_F14:         return KEYCODE_F14;
    case SDLK_F15:         return KEYCODE_F15;
#if SDL_VERSION_ATLEAST(1,3,0)
    case SDLK_F16:         return KEYCODE_F16;
    case SDLK_F17:         return KEYCODE_F17;
    case SDLK_F18:         return KEYCODE_F18;
    case SDLK_F19:         return KEYCODE_F19;
    case SDLK_F20:         return KEYCODE_F20;
    case SDLK_F21:         return KEYCODE_F21;
    case SDLK_F22:         return KEYCODE_F22;
    case SDLK_F23:         return KEYCODE_F23;
    case SDLK_F24:         return KEYCODE_F24;
#endif
    case SDLK_NUMLOCK:     return KEYCODE_NUMLOCK;
    case SDLK_SCROLLOCK:   return KEYCODE_SCROLL;
    //case SDLK_ASCIIBEGIN2: return KEYCODE_ASCIIBEGIN2;
    //case SDLK_ASCIIEND2:   return KEYCODE_ASCIIEND2;
    default:
        break;
    }
    return KEYCODE_UNKNOWN;
}
