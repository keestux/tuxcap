//
//  Preferences.m
//
//  Created by W.P. van Paassen on 8/15/11.
//

#import "Preferences.h"

#import <Foundation/Foundation.h>

void setDefault(const char* key, const char* value)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"%s", value] 
                                                            forKey:[NSString stringWithFormat:@"%s", key]
                                ];
    [defaults registerDefaults:appDefaults];
    [[NSUserDefaults standardUserDefaults] synchronize];

    [pool release];
}

void setUserDefault(const char* key, const char* value)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithFormat:@"%s", value] 
                                              forKey:[NSString stringWithFormat:@"%s", key] 
    ];
    [defaults synchronize];

    [pool release];
}

int getUserString(const char* key, unsigned char* buffer, int length)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    int retval = 0;  // assume failure
    if (str)
    {
        const char* string = [str UTF8String];
        strncpy((char*)buffer, string, length);
        retval = 1;
    }

    [pool release];
    return retval;
}

int getUserInteger(const char* key, unsigned char* buffer, int length)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString* str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    int retval = 0;  // assume failure
    if (str) {
        int i = atoi((const char*)[str UTF8String]);
        memcpy(buffer, &i, sizeof(int));
        retval = 1;
    }

    [pool release];
    return retval;
}

int getUserData(const char* key, unsigned char* buffer, int length)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString* str = [defaults stringForKey:[NSString stringWithFormat:@"%s", key]];
    int retval = 0;  // assume failure
    if (str) {
        NSData* data = [str dataUsingEncoding:NSUTF8StringEncoding];
        [data getBytes:(void*)buffer length:length];
        retval = 1;
    }

    [pool release];
    return retval;
}

void removeUserDefaultKey(const char* key)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    [[NSUserDefaults standardUserDefaults] synchronize];
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:[NSString stringWithFormat:@"%s", key]];   
    [[NSUserDefaults standardUserDefaults] synchronize];

    [pool release];
}

void removeUserDefaultValue(const char* value)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    [[NSUserDefaults standardUserDefaults] synchronize];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary* dict = [defaults dictionaryRepresentation];
    // TODO.
    // for (id anObject in [dict allKeys]) {
    //     ...
    // }
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

    [pool release];
}

void removeAllUserDefaults()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

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

    [pool release];
}
