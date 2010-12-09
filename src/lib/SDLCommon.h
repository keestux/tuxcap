/* 
 * File:   SDLCommon.h
 * Author: Kees Bakker
 *
 * Created on December 9, 2010, 9:42 PM
 */

#ifndef SDLCOMMON_H
#define	SDLCOMMON_H

#include "SexyAppBase.h"

namespace Sexy
{

class SDLException : public Exception
{
public:
    SDLException(const std::string & msg) : Exception(msg) {}
};


};


#endif	/* SDLCOMMON_H */

