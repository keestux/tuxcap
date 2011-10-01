//
//  Preferences.h
//  demo1
//
//  Created by Peter van Paassen on 8/22/11.
//

#ifndef demo1_Preferences_h
#define demo1_Preferences_h

#if __cplusplus
extern "C" {
#endif
    
    extern void setDefault(const char* key, const char* value);
    extern void setUserDefault(const char* key, const char* value);
    extern int getUserString(const char* key, unsigned char* buffer, int length);
    extern int getUserInteger(const char* key, unsigned char* buffer, int length);
    extern int getUserData(const char* key, unsigned char* buffer, int length);
    extern void removeUserDefaultKey(const char* key);
    extern void removeUserDefaultValue(const char* value);
    extern void removeAllUserDefaults();
    
#if __cplusplus
} //Extern C
#endif

#endif //demo1_Preferences_h
