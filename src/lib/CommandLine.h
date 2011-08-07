/*
 * File:   CommandLine.h
 * Author: Kees Bakker
 *
 * Created on December 15, 2010, 8:49 PM
 */

#ifndef COMMANDLINE_H
#define	COMMANDLINE_H

#include <string>
#include "anyoption.h"


namespace Sexy
{

// A singleton class the parse command line options, using anyoption
class CmdLine
{
public:
    static bool ParseCommandLine(int argc, char** argv);
    static std::string getArgv0() { return _cmdline ? _cmdline->_argv0 : ""; }
    static CmdLine * getCmdLine() { return _cmdline; }
    static AnyOption * getOpt() { return _cmdline ? _cmdline->_opt : NULL; }

private:
    CmdLine() : _argv0(), _opt(NULL) {}

    std::string         _argv0;
    AnyOption *         _opt;
    static CmdLine *    _cmdline;       // the only instance of the class, if any
};

}

#endif	/* COMMANDLINE_H */

