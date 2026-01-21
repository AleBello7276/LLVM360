
#include "Shared.h"

enum LogLevel
{
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR,
    LEVEL_DEBUG,
    LEVEL_FATAL
};

const char* getColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::LEVEL_INFO:    return "\033[32m"; // Green
    case LogLevel::LEVEL_WARNING: return "\033[33m"; // Yellow
    case LogLevel::LEVEL_ERROR:   return "\033[31m"; // Red
    case LogLevel::LEVEL_DEBUG:   return "\033[36m"; // Cyan
    case LogLevel::LEVEL_FATAL:   return "\033[35m"; // Magenta
    default: return "";
    }
}

void LOG_PRINT(LogLevel level, const char* name, const char* fmt, ...)
{
    printf("%s<%s> -> ", getColor(level), name);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\033[0m\n");
}

#define LOG_INFO(name, fmt, args...)    LOG_PRINT(LogLevel::LEVEL_INFO,    name, fmt, ## args)
#define LOG_WARNING(name, fmt, args...) LOG_PRINT(LogLevel::LEVEL_WARNING, name, fmt, ## args)
#define LOG_ERROR(name, fmt, args...)   LOG_PRINT(LogLevel::LEVEL_ERROR,   name, fmt, ## args)
#define LOG_DEBUG(name, fmt, args...)   LOG_PRINT(LogLevel::LEVEL_DEBUG,   name, fmt, ## args)
#define LOG_FATAL(name, fmt, args...)   LOG_PRINT(LogLevel::LEVEL_FATAL,   name, fmt, ## args)


