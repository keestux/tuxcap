/* 
 * File:   Logging.h
 * Author: Kees Bakker
 *
 * Created on December 9, 2010, 9:53 PM
 * Copied level names from Pantheios.
 */

#ifndef LOGGING_H
#define	LOGGING_H

#include <string>
#include <cstdio>

namespace Sexy
{

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

class Logger
{
public:
    Logger() {}
    virtual ~Logger() { }

    enum LOG_LEVEL
    {
        LVL_EMERGENCY       =   0,   /*!< system is unusable */
        LVL_ALERT           =   1,   /*!< action must be taken immediately */
        LVL_CRITICAL        =   2,   /*!< critical conditions */
        LVL_ERROR           =   3,   /*!< error conditions */
        LVL_WARNING         =   4,   /*!< warning conditions */
        LVL_NOTICE          =   5,   /*!< normal but significant condition */
        LVL_INFORMATIONAL   =   6,   /*!< informational */
        LVL_DEBUG           =   7,   /*!< debug-level messages */
    } ;
    static void set_log_level(enum LOG_LEVEL lvl);
    static std::string level_txt(enum LOG_LEVEL lvl);
    static void log(enum LOG_LEVEL lvl, const char * txt);
    static void log(enum LOG_LEVEL lvl, const std::string & txt);
private:
    virtual void write_str(const char * txt) const = 0;
    virtual void write_str(const std::string & txt) const = 0;

    enum LOG_LEVEL      _level;

protected:
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

}

#endif	/* LOGGING_H */

