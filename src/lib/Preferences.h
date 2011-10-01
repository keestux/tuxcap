//
//  Preferences.h
//
//  Created by Peter van Paassen on 8/22/11.
//

#ifndef _Preferences_h
#define _Preferences_h

#if __cplusplus
extern "C" {
#endif
    
extern void setDefault(const char* key, const char* value);
extern void setUserDefault(const char* key, const char* value);
extern int getUserString(const char* key, unsigned char* buffer, int length);
extern int getUserInteger(const char* key, unsigned char* buffer, int length);
extern int getUserData(const char* key, unsigned char* buffer, int length);
extern void removeUserDefaultKey(const char* key);
extern void removeAllUserDefaults();
    
#if __cplusplus
} //Extern C
#endif

#endif // _Preferences_h
