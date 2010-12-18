//
// Logging.cpp
// A simple class to do logging and diagnostic
//

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>

#include "Logging.h"
#include "Timer.h"

using namespace std;

// TODO. Make this a vector, and write the log entries to all of
// them. And then add special loggers for really important (error) messages,
Logger * Logger::_logger;
Timer * Logger::_timer;
double Logger::_start_time;

Logger::Logger()
{
    // One timer for all loggers
    if (_timer == NULL) {
        _timer = new Timer();
        _timer->start();
        _start_time = _timer->getElapsedTimeInSec();
    }

    // Create a few essential logging facilities
    // TODO
}

bool Logger::set_log_level(const string & txt)
{
    // txt has the following syntax:
    // LEVEL [ ',' LEVEL ]
    // where LEVEL is <name> [ '=' <level> ]
    // where <name> is a string and <level> is an integer
    // In other words the <level> is optional. Default: 1
    string mytxt = txt;
    while (mytxt != "") {
        size_t cpos = mytxt.find(',');
        string level;
        if (cpos != string::npos) {
            level = mytxt.substr(0, cpos);
        }
        else {
            level = mytxt;
        }

        size_t epos = level.find('=');
        string name;
        int num = 0;
        if (epos != string::npos) {
            name = level.substr(0, epos);
            num = strtol(level.substr(epos + 1).c_str(), NULL, 0);
        }
        else {
            name = level;
            num = 1;        // Default: 1
        }
        if (LoggerFacil::find(name)) {
            // Already exists
            // ???? Throw exception?
        }
        else {
            LoggerFacil::add(name, num);
        }

        if (cpos != string::npos) {
            mytxt = mytxt.substr(cpos + 1);
        }
        else {
            mytxt = "";
        }
    }

    return true;
}

void Logger::log(LoggerFacil * facil, int lvl, const char * txt)
{
    if (_logger == NULL || facil == NULL) {
        return;
    }
    if (lvl > facil->getLevel()) {
        // The detail level of this log entry is higher than the max requested level.
        return;
    }
    _logger->write_str(facil->getName() + ": " + txt);
}

void Logger::log(LoggerFacil * facil, int lvl, const string & txt)
{
    log(facil, lvl, txt.c_str());
}

void Logger::tlog(LoggerFacil * facil, int lvl, const char * txt)
{
    if (_logger == NULL || facil == NULL) {
        return;
    }
    if (lvl > facil->getLevel()) {
        // The level of this log entry is higher than the maximum and thus not wanted.
        return;
    }
    char tmp[20];
    sprintf(tmp, "%8.3f ", _timer->getElapsedTimeInSec() - _start_time);
    _logger->write_str(string(tmp) + facil->getName() + ": " + txt);
}

void Logger::tlog(LoggerFacil * facil, int lvl, const string & txt)
{
    tlog(facil, lvl, txt.c_str());
}

// Utility function to quote a text
string Logger::quote(const string & str, char qu)
{
    return string("") + qu + str + qu;
}

string Logger::vformat(const char* fmt, va_list argPtr)
{
    // If the string is less than 161 characters,
    // allocate it on the stack because this saves
    // the malloc/free time.
    const int bufSize = 161;
    char stackBuffer[bufSize];

    int attemptedSize = bufSize - 1;

    int numChars = 0;
#ifdef _WIN32
    numChars = _vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#else
    numChars = vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);
#endif

    if ((numChars >= 0) && (numChars <= attemptedSize)) {
        // Needed for case of 160-character printf thing
        stackBuffer[numChars] = '\0';

        // Got it on the first try.
        return string(stackBuffer);
    }

    // Now use the heap.
    char* heapBuffer = NULL;

    // We draw the line at a 1MB string.
    const int maxSize = 1000000;

    while (((numChars == -1) || (numChars > attemptedSize)) && (attemptedSize < maxSize)) {
        // Try a bigger size
        attemptedSize *= 2;
        heapBuffer = (char*)realloc(heapBuffer, (attemptedSize + 1));
#ifdef _WIN32
        numChars = _vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#else
        numChars = vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
#endif
    }

    // Terminate the string
    heapBuffer[numChars] = 0;

    string result = string(heapBuffer);

    free(heapBuffer);

    return result;
}

std::string Logger::format(const char* fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    std::string result = vformat(fmt, argList);
    va_end(argList);

    return result;
}

StdoutLogger::StdoutLogger() : Logger()
{
    // Nothing to do.
}

void StdoutLogger::create_logger()
{
    _logger = new StdoutLogger();
}

void StdoutLogger::write_str(const char * txt) const
{
    int len = strlen(txt);
    if (len == 0) {
        return;
    }

    fputs(txt, stdout);
    // ???? Each string must end with newline?
    if (txt[len-1] != '\n') {
        fputc('\n', stdout);
    }
    fflush(stdout);
}

void StdoutLogger::write_str(const string & txt) const
{
    write_str(txt.c_str());
}

FileLogger::FileLogger(const char * fname) : Logger()
{
    _fp = fopen(fname, "a");
    if (_fp == NULL) {
        throw new LoggerException("Cannot open log file: '" + string(fname) + "'");
    }
}

FileLogger::FileLogger(const string & fname)
{
    _fp = fopen(fname.c_str(), "a");
    if (_fp == NULL) {
        throw new LoggerException("Cannot open log file: '" + fname + "'");
    }
}

void FileLogger::create_logger(const char * fname)
{
    _logger = new FileLogger(fname);
}

void FileLogger::create_logger(const string & fname)
{
    _logger = new FileLogger(fname);
}

void FileLogger::write_str(const char * txt) const
{
    if (_fp == NULL) {
        return;
    }

    int len = strlen(txt);
    if (len == 0) {
        return;
    }

    fputs(txt, _fp);
    // ???? Each string must end with newline?
    if (txt[len-1] != '\n') {
        fputc('\n', _fp);
    }
     fflush(_fp);
}

void FileLogger::write_str(const string & txt) const
{
    write_str(txt.c_str());
}

std::map<const std::string, LoggerFacil *>  LoggerFacil::_all_facils;
LoggerFacil * LoggerFacil::find(const std::string& name)
{
    std::map<const std::string, LoggerFacil *>::const_iterator it;
    it = _all_facils.find(name);
    if (it == _all_facils.end()) {
        return NULL;
    }
    return it->second;
}

void LoggerFacil::add(const std::string& name, int min_level)
{
    if (LoggerFacil::find(name)) {
        // Don't add another with the same name
        return;
    }
    LoggerFacil * facil = new LoggerFacil(name, min_level);
    _all_facils[name] = facil;
}
