//
//  Preferences.m
//
//  Created by W.P. van Paassen on 8/15/11.
//

#import "Preferences.h"

#import <Foundation/Foundation.h>

void setDefault(const char* key, const char* value)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"%s", value] 
                                                            forKey:[NSString stringWithFormat:@"%s", key]
                                ];
    [defaults registerDefaults:appDefaults];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void setUserDefault(const char* key, const char* value)
{
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithFormat:@"%s", value] 
                                              forKey:[NSString stringWithFormat:@"%s", key] 
    ];
    [defaults synchronize];
}

int getUserString(const char* key, unsigned char* buffer, int length)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    if (str == nil)
    {
        return 0;
    }
    const char* string = [str UTF8String];
    strncpy((char*)buffer, string, length);
    return 1;
}

int getUserInteger(const char* key, unsigned char* buffer, int length)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    NSString* str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    if (str == nil) {
        return 0;
    }
    int i = atoi((const char*)[str UTF8String]);
    memcpy(buffer, &i, sizeof(int));
    return 1;
}

int getUserData(const char* key, unsigned char* buffer, int length)
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString* str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    if (str == nil) {
        return 0;
    }
    NSData* data = [str dataUsingEncoding:NSUTF8StringEncoding];
    [data getBytes:(void*)buffer length:length];
    return 1;
}

void removeUserDefaultKey(const char* key)
{
    [[NSUserDefaults standardUserDefaults] synchronize];
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:[NSString stringWithFormat:@"%s", key]];   
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void removeUserDefaultValue(const char* value)
{
    [[NSUserDefaults standardUserDefaults] synchronize];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary* dict = [defaults dictionaryRepresentation];
    NSArray* array = [dict allKeys];
    NSEnumerator* en = [array objectEnumerator];
    id anObject;
    while (anObject = [en nextObject]) {
        NSString* str = [dict valueForKey:anObject];
        if ([str UTF8String] == value) {
            [defaults removeObjectForKey:anObject];
        }
    }
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void removeAllUserDefaults()
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults synchronize];
    NSDictionary* dict = [defaults dictionaryRepresentation];
    NSArray* array = [dict allKeys];
    NSEnumerator* en = [array objectEnumerator];
    id anObject;
    while (anObject = [en nextObject]) {
            [defaults removeObjectForKey:anObject];
    }
    [defaults synchronize];
}
