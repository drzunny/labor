#ifndef __LABOR_LOG_H__
#define __LABOR_LOG_H__

#include <memory>
#include <string>


#define LOG_DEBUG \
    (labor::Logger(labor::Logger::LV_DEBUG, __FILE__, __LINE__).write)

#define LOG_INFO \
    (labor::Logger(labor::Logger::LV_INFO, __FILE__, __LINE__).write)

#define LOG_WARNING \
    (labor::Logger(labor::Logger::LV_WARNING, __FILE__, __LINE__).write)

#define LOG_ERROR \
    (labor::Logger(labor::Logger::LV_ERROR, __FILE__, __LINE__).write)

#endif

namespace labor
{

    // FIXIT: `Logger` class is not a good design now..
    class Logger
    {
    public:
        enum LoggerLevel
        {
            LV_DEBUG = 0,
            LV_INFO = 1,
            LV_WARNING = 2,
            LV_ERROR = 3
        };

    public:
        Logger(LoggerLevel level, const char * filename, int line);
        void write(const std::string & content, ...);

    private:
        static std::string filepath_;
        static std::string format_;
        static int maxsize_;
        static bool merge_;

        const char * source_;
        int line_;
        LoggerLevel level_;
        
    };
}
