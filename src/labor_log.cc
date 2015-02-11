#include "labor_log.h"
#include "labor_utils.h"
#include "labor_def.h"

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
_now_string() {
    uint64_t now = labor::timestamp_now();
    return "";
}


static string
_logger_level_string(labor::Logger::LoggerLevel level)  {
    switch (level)
    {
    case labor::Logger::LV_DEBUG:
        return "[ DEBUG ]";
    case labor::Logger::LV_WARNING:
        return "[WARNING]";
    case labor::Logger::LV_ERROR:
        return "[ ERROR ]";
    default:
        return "[ INFO ] ";
    }
}


/* ------------------------------------
* logger operation implements
* ------------------------------------
*/

struct _logger_struct {
    char text[LABOR_LOG_BUFFER];
    char logpath[256];
    char filepath[256];
    int line;
    int level;
};

static bool _logger_isstartup = false;
static pthread_t _logger_thread;
static queue<_logger_struct> _logger_queue;
static atomic<bool> _logger_lock = ATOMIC_VAR_INIT(false);  // use spinlock to wait

static void
_logger_queue_write(const _logger_struct * log)  {
    printf("%s\n", log->text);
    // TODO: write file
}


static string
_logger_queue_datefile(const char * fixpath) {
    return "";
}


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
            // this operation will unlock the spinlock, it equal to `break`
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
        auto log = _logger_queue.front();
        _logger_queue_write(&log);
        _logger_queue.pop();
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
_logger_queue_push(const string & logpath, int level,
const string & filename, int line, const string & content)    {
    _logger_struct ls;
    auto trueFile = _logger_queue_datefile(logpath.c_str());
    memcpy(ls.logpath, trueFile.c_str(), trueFile.length() + 1);
    memcpy(ls.filepath, filename.c_str(), filename.length() + 1);
    memcpy(ls.text, content.c_str(), content.length() + 1);
    ls.line = line;
    ls.level = level;
    
    _logger_queue.push(ls);

    // if push success, resume the log thread immediately.
    _logger_queue_resume();
}


/* ------------------------------------
* The class implement
* ------------------------------------
*/
string labor::Logger::filepath_ = _load_config("log.file_path", "./");
string labor::Logger::format_ = _load_config("log.format", "@content");
int labor::Logger::maxsize_ = _string2int(_load_config("log.file_size", "10"));
bool labor::Logger::merge_ = _string2bool(_load_config("log.merge", "1"));


labor::Logger::Logger(labor::Logger::LoggerLevel level, const char * filename, int line)
    : level_(level), source_(filename), line_(line) {
    _logger_queue_start();
}


void
labor::Logger::write(const char * content, ...) {
    char log_buffer[LABOR_LOG_BUFFER];
    memset(log_buffer, 0, LABOR_LOG_BUFFER);
    va_list vars;
    va_start(vars, content);
    vsprintf(log_buffer, content, vars);
    va_end(vars);
    _logger_queue_push(this->filepath_, (int)level_, source_, line_, __S(log_buffer));
}
