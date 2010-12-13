#include <cstdio>
#include <string>

#include "Logging.h"

using namespace std;

namespace Sexy
{

Logger * Logger::_logger;

void Logger::set_log_level(enum LOG_LEVEL lvl)
{
    if (_logger) {
        switch (lvl) {
        case LVL_EMERGENCY:
        case LVL_ALERT:
        case LVL_CRITICAL:
        case LVL_ERROR:
        case LVL_WARNING:
        case LVL_NOTICE:
        case LVL_INFORMATIONAL:
        case LVL_DEBUG:
            _logger->_level = lvl;
            break;
        default:
            // ???? Throw exception?
            _logger->_level = LVL_ERROR;
            break;
        }
    }
    Logger::log(Logger::LVL_DEBUG, string("Logger::set_log_level => ") + level_txt(lvl));
}

string Logger::level_txt(enum LOG_LEVEL lvl)
{
    switch (lvl) {
    case LVL_EMERGENCY: return "EMERGENCY";
    case LVL_ALERT: return "ALERT";
    case LVL_CRITICAL: return "CRITICAL";
    case LVL_ERROR: return "ERROR";
    case LVL_WARNING: return "WARNING";
    case LVL_NOTICE: return "NOTICE";
    case LVL_INFORMATIONAL: return "INFORMATIONAL";
    case LVL_DEBUG: return "DEBUG";
    }
    return "???";
}

void Logger::log(enum LOG_LEVEL lvl, const char * txt)
{
    if (_logger == NULL) {
        return;
    }
    if (lvl > _logger->_level) {
        // The level of this log entry is higher than the required level and thus not wanted.
        return;
    }
    _logger->write_str(txt);
}

void Logger::log(enum LOG_LEVEL lvl, const std::string & txt)
{
    log(lvl, txt.c_str());
}

StdoutLogger::StdoutLogger()
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

FileLogger::FileLogger(const char * fname)
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

}
