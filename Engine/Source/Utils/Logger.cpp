#include "Logger.h"

#if HIGHLIGHTING_LOGS
#   include "termcolor/include/termcolor/termcolor.hpp"
using namespace termcolor;
#endif //HIGHLIGHTING_LOGS

#ifdef PLATFORM_ANDROID
#   include "Platform/Android/WindowAndroid.h"

extern void android_log_arg(v3d::utils::Logger::LoggerType type, const char* message, va_list ap);
extern void android_log(v3d::utils::Logger::LoggerType type, const char* message, ...);
#endif //PLATFORM_ANDROID

namespace v3d
{
namespace utils
{

const std::string Logger::s_loggerType[Logger::LoggerCount] =
{
    "DEBUG",
    "",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

#if HIGHLIGHTING_LOGS
using colorFunc = std::ostream& (*)(std::ostream& stream);

const colorFunc k_colorList[Logger::LoggerCount] =
{
    green,
    white,
    white,
    yellow,
    red,
    magenta
};
#endif //HIGHLIGHTING_LOGS

Logger::Logger() noexcept
    : m_logFilename("logfile.log")
#if defined(DEBUG) && !defined(DEVELOPMENT)
    , m_level(LoggerType::LoggerDebug)
#else //defined(DEBUG)
    , m_level(LoggerType::LoggerNotify)
#endif //defined(DEBUG)
    , m_immediateFlush(true)
{
    m_fileBuffer.reserve(k_bufferSize);

    m_file.open(m_logFilename, std::ofstream::out);
    m_file.clear();
    m_file.close();
}

Logger::~Logger()
{
    logFlushToFile();

    m_fileBuffer.clear();
    m_file.close();
}

void Logger::createLogFile(const std::string& fileName)
{
    std::lock_guard lock(m_mutex);

    m_logFilename = fileName;

    m_file.open(m_logFilename, std::ofstream::out);
    m_file.clear();
    m_file.close();
}

void Logger::log(LoggerType type, u16 maskOut, const char* format, ...)
{
    std::lock_guard lock(m_mutex);

    if (m_level > type)
    {
        return;
    }

#ifdef PLATFORM_ANDROID
    va_list args;
    va_start(args, format);
    android_log_arg(type, format, args);
    va_end(args);
#else
    char buffer[k_maxMessageSize];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string message;
    message.assign(buffer);

    log(message, type, maskOut);
#endif
}

void Logger::flush()
{
    std::lock_guard lock(m_mutex);

    logFlushToFile();
}

void Logger::setLogLevel(LoggerType type)
{
    std::lock_guard lock(m_mutex);

    m_level = type;
}

void Logger::setImmediateFlushToFile(bool immediate)
{
    std::lock_guard lock(m_mutex);

    m_immediateFlush = immediate;
}

void Logger::log(const std::string& message, LoggerType type, u16 maskOut)
{
    std::lock_guard lock(m_mutex);

    if (m_level > type)
    {
        return;
    }

    if (maskOut & LogOut::ConsoleLog)
    {
#ifdef PLATFORM_ANDROID
        android_log(type, message.c_str());
#else   
        logToConsole(message, type);
#endif
    }

    if (maskOut & LogOut::FileLog)
    {
        if (m_immediateFlush)
        {
            logToFile(message, type);
        }
        else
        {
            if (m_fileBuffer.size() == k_bufferSize)
            {
                logFlushToFile();
            }

            std::string temp(s_loggerType[type]);
            if (type != Logger::LoggerNotify)
            {
                temp.append(": ");
            }
            temp.append(message);

            m_fileBuffer.push_back(temp);
        }
    }
}

void Logger::destroyLogFile()
{
    std::lock_guard lock(m_mutex);

    if (m_file.is_open())
    {
        m_file.close();
    }
    std::remove(m_logFilename.c_str());
}

void Logger::logToConsole(const std::string& message, LoggerType type)
{
    if (type == Logger::LoggerNotify)
    {
#if HIGHLIGHTING_LOGS
        std::cout << k_colorList[type] << s_loggerType[type] << message << reset << std::endl;
#else //HIGHLIGHTING_LOGS
        std::cout << s_loggerType[type] << message << std::endl;
#endif //HIGHLIGHTING_LOGS
    }
    else
    {
#if HIGHLIGHTING_LOGS
        std::cout << k_colorList[type] << s_loggerType[type] << ": " << message << reset << std::endl;
#else //HIGHLIGHTING_LOGS
        std::cout << s_loggerType[type] << ": " << message << std::endl;
#endif //HIGHLIGHTING_LOGS
    }

}

void Logger::logToFile(const std::string& message, LoggerType type)
{
    m_file.open(m_logFilename, std::ofstream::out | std::ofstream::app);
    if (type == Logger::LoggerNotify)
    {
        m_file << s_loggerType[type] << message << std::endl;
    }
    else
    {
        m_file << s_loggerType[type] << ": " << message << std::endl;
    }
    m_file.close();
}

void Logger::logFlushToFile()
{
    if (m_fileBuffer.empty())
    {
        return;
    }

    m_file.open(m_logFilename, std::ofstream::out | std::ofstream::app);
    for (std::string& message : m_fileBuffer)
    {
        m_file << message << std::endl;
    }
    m_file.close();

    m_fileBuffer.clear();
}

} //namespace utils
} //namespace v3d
