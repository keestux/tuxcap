/* 
 * File:   Logging.h
 * Author: Kees Bakker
 *
 * Created on December 9, 2010, 9:53 PM
 */

#ifndef LOGGING_H
#define	LOGGING_H

#include <string>
#include <map>
#include <cstdio>
#include <cstdarg>
#include "Timer.h"

class LoggerException : std::exception
{
public:
    LoggerException(const std::string & msg)
        : std::exception(),
        mMessage(msg)
        {}
    virtual ~LoggerException() throw (){}

    virtual const std::string     getMessage() const    { return mMessage; }
private:
    std::string     mMessage;
};

class LoggerFacil
{
public:
    static LoggerFacil * find(const std::string & name);
    static void add(const std::string & name, int min_level);
    int getLevel() const { return _level; }
    std::string getName() const { return _name; }
private:
    LoggerFacil(const std::string & name, int level) : _name(name), _level(level) {}
    std::string _name;
    int         _level;

    static std::map<const std::string, LoggerFacil *>  _all_facils;
};

class Logger
{
public:
    Logger();
    virtual ~Logger() {}

    static bool set_log_level(const std::string & txt);
    static void log(LoggerFacil * facil, int lvl, const char * txt);
    static void log(LoggerFacil * facil, int lvl, const std::string & txt);
    // tlog also writes the timer value
    static void tlog(LoggerFacil * facil, int lvl, const char * txt);
    static void tlog(LoggerFacil * facil, int lvl, const std::string & txt);
    static std::string quote(const std::string& str, char qu='"');
    static std::string format(const char* fmt, ...);
private:
    virtual void write_str(const char * txt) const = 0;
    virtual void write_str(const std::string & txt) const = 0;
    static std::string vformat(const char* fmt, va_list argPtr);

protected:
    static Timer *  _timer;
    static double   _start_time;
    static Logger * _logger;
};

class StdoutLogger : Logger
{
public:
    static void create_logger();
private:
    StdoutLogger();
    void write_str(const char * txt) const;
    void write_str(const std::string & txt) const;
};

class FileLogger : Logger
{
public:
    static void create_logger(const char * fname);
    static void create_logger(const std::string & fname);
private:
    FileLogger(const char * fname);
    FileLogger(const std::string & fname);
    void write_str(const char * txt) const;
    void write_str(const std::string & txt) const;

    const char *    _fname;
    FILE *          _fp;
};

#endif	/* LOGGING_H */

