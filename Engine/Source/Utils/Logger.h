#pragma once

#include "Common.h"
#include "Singleton.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class Logger final : public Singleton<Logger>
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        enum LoggerType
        {
            LoggerDebug,
            LoggerNotify,
            LoggerInfo,
            LoggerWarning,
            LoggerError,
            LoggerFatal,

            LoggerCount
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        enum LogOut : u16
        {
            ConsoleLog = 0x01,
            FileLog = 0x02,
        };

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        void                                    setLogLevel(LoggerType type);

        void                                    log(const std::string& message, LoggerType type = LoggerType::LoggerInfo, u16 maskOut = LogOut::ConsoleLog);
        void                                    log(LoggerType type, u16 maskOut, const char* format, ...);
        void                                    flush();

        void                                    setImmediateFlushToFile(bool immediate);
        void                                    createLogFile(const std::string& filename);
        void                                    destroyLogFile();

    private:

        friend Singleton<Logger>;
        Logger();
        ~Logger();

    protected:

        void                                    logToConsole(const std::string& message, LoggerType type);
        void                                    logToFile(const std::string& message, LoggerType type);
        void                                    logFlushToFile();

        std::string                             m_logFilename;
        std::ofstream                           m_file;

        LoggerType                              m_level;

        const static u32                        k_bufferSize = 32;
        const static u32                        k_maxMessageSize = 1024;

        bool                                    m_immediateFlush;
        std::vector<std::string>                m_fileBuffer;

        const static std::string                s_loggerType[Logger::LoggerCount];
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOGGER
#   define LOG_DEBUG(messages, ...)     v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerDebug, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);
#   define LOG(messages, ...)           v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerNotify, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);
#   define LOG_INFO(messages, ...)      v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerInfo, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);
#   define LOG_WARNING(messages, ...)   v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerWarning, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);
#   define LOG_ERROR(messages, ...)     v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerError, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);
#   define LOG_FATAL(messages, ...)     v3d::utils::Logger::getInstance()->log(v3d::utils::Logger::LoggerFatal, v3d::utils::Logger::ConsoleLog, messages, ##__VA_ARGS__);

#else //USE_LOGGER
#   define LOG_DEBUG(messages, ...)
#   define LOG(messages, ...)
#   define LOG_INFO(messages, ...)
#   define LOG_WARNING(messages, ...)
#   define LOG_ERROR(messages, ...)
#   define LOG_FATAL(messages, ...)
#endif //USE_LOGGER

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d