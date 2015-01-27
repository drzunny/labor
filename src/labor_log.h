#ifndef __LABOR_LOG_H__
#define __LABOR_LOG_H__

#include <memory>
#include <string>

namespace labor
{
    class Logger
    {
    public:
        enum LoggerLevel
        {
            DEBUG = 0,
            INFO = 1,
            WARNING = 2,
            ERR = 3
        };

        static std::weak_ptr<Logger> defaultLogger();

    public:
        Logger(const std::string & filepath, const std::string & formatter, size_t maxsize, bool merge);
        void write(LoggerLevel level, const std::string & filename, int line, const std::string & content, ...);

    private:
        std::string filepath_;
        std::string format_;
        int maxsize_;
        bool enableMerge_;
        
        static std::shared_ptr<Logger> defaultLogger_;
    };
}

#define LOG_INIT() auto logger = labor::Logger::defaultLogger().lock();

#define LOG_DEBUG(x) \
    logger->write(labor::Logger::DEBUG, __FILE__, __LINE__, ##x##)

#define LOG_INFO(x) \
    logger->write(labor::Logger::INFO, __FILE__, __LINE__, ##x##)

#define LOG_WARNING(x) \
    logger->write(labor::Logger::WARNING, __FILE__, __LINE__, ##x##)

#define LOG_ERROR(x) \
    logger->write(labor::Logger::ERROR, __FILE__, __LINE__, ##x##)

#endif