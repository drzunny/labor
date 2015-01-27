#include "labor_log.h"
#include "labor_utils.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include <queue>
#include <atomic>
#include <boost/algorithm/string.hpp>

#ifdef WIN32
#   include <Windows.h>
#else
#   include <stdlib.h>
#endif

using namespace std;

/*
*   TODO.   i want to use other logger library to instead my `Logger` implement.
            but all of them are required fully C++11 support...
            my production environment's GCC is 4.4.7
            so, i just use POSIX Thread to implement it by myself.
*/


/* ------------------------------------
* The helpers
* ------------------------------------
*/
static int
_string2int(const string && s)   {
    return 0;
}


static bool
_string2bool(const string && s) {
    return false;
}


static string
_format_string(const string & content, va_list args) {
    return "";
}


static inline string
_load_config(const string & name, const string & dval = "")   {
    return labor::readConfig(name, dval);
}

static string
_today_string() {
    uint64_t now = labor::timestamp_now();
    return "";
}


static string
_datetime_utc_now() {
    uint64_t utc_now = labor::timestamp_now();
    return "";
}

static string
_logger_level_string(labor::Logger::LoggerLevel level)  {
    switch (level)
    {
    case labor::Logger::DEBUG:
        return "[ DEBUG ]";
    case labor::Logger::WARNING:
        return "[WARNING]";
    case labor::Logger::ERR:
        return "[ ERROR ]";
    default:
        return "[ INFO ] ";
    }
}


/* ------------------------------------
* logger operation implements
* ------------------------------------
*/

static bool _logger_isstartup = false;
static pthread_t _logger_thread;
static queue<string> _logger_queue;
static atomic<bool> _logger_lock = ATOMIC_VAR_INIT(false);  // use spinlock to wait

static void
_logger_queue_resume() {
    bool isLock = false;
    bool check = std::atomic_exchange(&_logger_lock, isLock);
    if (check)
        printf("[WARNNING]  Atomic fail to release the lock!");
}


static void
_logger_queue_wait(size_t secs)   {
    static uint64_t ts = labor::timestamp_now();

    // To avoid the idle-loop to waste the CPU. use sleep(0)
    while (std::atomic_load(&_logger_lock) == true)
    {
#if WIN32
        Sleep(0);
#else
        sleep(0);
#endif
        uint64_t now = labor::timestamp_now();
        // if timeout, unlock
        if (now - ts >= secs * 1000)
        {
            ts = now;
            _logger_queue_resume();
        }
    }
}

static inline bool
_logger_queue_has() {
    return _logger_queue.size() > 0;
}


static void *
_logger_queue_handler(void * args) {
    while (true)
    {
        if (!_logger_queue_has())
        {
            _logger_queue_wait(30);
            continue;
        }
        // write
    }
}


static void
_logger_queue_start()   {
    if (_logger_isstartup)
        return;

    // Create the Logger thread
    int ret = pthread_create(&_logger_thread, NULL, _logger_queue_handler, NULL);
    BOOST_ASSERT_MSG(ret != 0, "Create Logger thread error");

    _logger_isstartup = true;
}


static void
_logger_queue_push(const string & filepath, int level, 
                   const string & filename, int line, const string & content, ...)    {

    // if push success, resume the log thread immediately.
    _logger_queue_resume();
}


/* ------------------------------------
* The class implement
* ------------------------------------
*/
shared_ptr<labor::Logger>
labor::Logger::defaultLogger_ = shared_ptr<labor::Logger>(NULL);


weak_ptr<labor::Logger>
labor::Logger::defaultLogger()  {
    if (labor::Logger::defaultLogger_.get() == NULL)
    {
        string && filepath = _load_config("file_path", "./");
        int maxFileSize = _string2int(_load_config("file_size", "10"));
        string && format = _load_config("format", "@content");
        bool isMerge = _string2bool(_load_config("merge", "1"));

        labor::Logger::defaultLogger_.reset(new labor::Logger(filepath, format, maxFileSize, isMerge));
    }
    return weak_ptr<labor::Logger>(labor::Logger::defaultLogger_);
}


labor::Logger::Logger(const string & filepath, const string & formatter, size_t maxsize, bool merge) 
    : filepath_(filepath), format_(formatter), maxsize_(maxsize), enableMerge_(merge)
{
}


void
labor::Logger::write(labor::Logger::LoggerLevel level, const string & filename, int line, const string & content, ...) {
    _logger_queue_push(this->filepath_, (int)level, filename, line, content);
}
