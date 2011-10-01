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
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"%s", value] 
                                                            forKey:myKey
                                ];
    [defaults registerDefaults:appDefaults];
    [defaults synchronize];

    [pool release];
}

void setUserDefault(const char* key, const char* value)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    [defaults setObject:[NSString stringWithFormat:@"%s", value] 
                 forKey:myKey 
    ];
    [defaults synchronize];

    [pool release];
}

int getUserString(const char* key, unsigned char* buffer, int length)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    NSString *str = [defaults stringForKey:myKey];
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
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    NSString* str = [defaults stringForKey:myKey];
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
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    NSString* str = [defaults stringForKey:myKey];
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

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *myKey = [NSString stringWithFormat:@"%s", key];
    [defaults synchronize];
    [defaults removeObjectForKey:myKey];
    [defaults synchronize];

    [pool release];
}

void removeAllUserDefaults()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults synchronize];
    NSDictionary* dict = [defaults dictionaryRepresentation];
    for (id anObject in [dict allKeys]) {
        NSLog(@"removeAllUserDefaults - removing '%@'", anObject);
        // ???? For now, do not remove
        //[defaults removeObjectForKey:anObject];
    }
    [defaults synchronize];

    [pool release];
}
