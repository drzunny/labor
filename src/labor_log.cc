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

#ifdef LABOR_DEBUG
# define __DBG_SAY(x) if (labor::Logger::enableStdout()) {printf(x);}
#else
# define __DBG_SAY(x)
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
    return true;
}


static inline string
_load_config(const string & name, const string & dval = "")   {
    return labor::readConfig(name, dval);
}


static string
_logger_level_string(int iLevel)  {
    labor::Logger::LoggerLevel level = (labor::Logger::LoggerLevel)iLevel;
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


static inline bool
_logger_file_exists(const char * filepath)  {
    FILE * f = fopen(filepath, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}


static inline FILE *
_logger_file_reopen(FILE * old, const char * filepath)  {
    fclose(old);
    return fopen(filepath, "a");
}


static inline const char *
_logger_strip_source_filename(const char * source)  {
    // strip the base file name, ex: /../../a.cc ==> a.cc
    int len = strlen(source);
    int pos = len - 1;
    const char * c = source + pos;
    while (c >= source && (*c != '\\' && *c != '/')) { c--; }
    // c = source - 1 now
    c += 1;
    return c;
}

/* ------------------------------------
* logger operation implements
* ------------------------------------
*/

struct _log_body_t {
    char text[LABOR_LOG_BUFFER];
    char logpath[256];
    char filepath[32];
    int line;
    int level;
};


static bool _logger_isstartup = false;
static pthread_t _logger_thread;
static deque<_log_body_t> _logger_queue;

// the spin-locks
static atomic<bool> _logger_lock_wait = ATOMIC_VAR_INIT(true);


/*
*  write log to stdout and file
*/
static void
_logger_queue_write(const _log_body_t * log)  {
    static string s_logfile_path = "";
    static FILE * s_logfile_handle = NULL;

    string text = "$level>> $file | $line | $datetime | $text";
    int level = log->level;
    if (!labor::Logger::format().empty())
        text = labor::Logger::format();

    labor::string_replace(text, __S("$level"), _logger_level_string(level));
    labor::string_replace(text, __S("$file"), log->filepath);
    labor::string_replace(text, __S("$line"), std::to_string(log->line));
    labor::string_replace(text, __S("$datetime"), labor::time_now_string());
    labor::string_replace(text, __S("$text"), log->text);

    if (labor::Logger::enableStdout())
        printf("%s\n", text.c_str());

    // Write to file
    // if log file has been changed, close the old FILE and reopen the new one.
    if (s_logfile_path.compare(log->logpath) != 0)  {
        if (s_logfile_handle != NULL)   {
            fclose(s_logfile_handle);
        }

        s_logfile_path = log->logpath;
        if (!_logger_file_exists(log->logpath))
            s_logfile_handle = fopen(log->logpath, "w");
        else
            s_logfile_handle = fopen(log->logpath, "a");

        LABOR_ASSERT(s_logfile_handle != NULL, "cannot open logfile");
    }
    fprintf(s_logfile_handle, "%s\n", text.c_str());
    fflush(s_logfile_handle);
}


/*
*  the log file name with date string
*/
static string
_logger_queue_datefile() {
    string s = labor::Logger::filePath();
    char datestr[100] = { 0 };
    time_t now = time(NULL);
    strftime(datestr, 10, ".%Y%m%d", localtime(&now));

    s.append("labor.log");
    return s.append(datestr);
}


/*
*  resume the logger-thread
*/
static void
_logger_queue_resume() {
    const bool isLock = false;
    bool check = std::atomic_exchange(&_logger_lock_wait, isLock);
    if (check)
        printf("[WARNNING]  Atomic fail to release the lock!");
}


/*
*  a spinlock and timer for waiting the log arrived
*/
static void
_logger_queue_wait(size_t secs)   {
    static uint64_t ts = labor::timestamp_now();

    // To avoid the idle-loop to waste the CPU. use sleep(0)
    while (std::atomic_load(&_logger_lock_wait) == true)
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


/*
*  check the logger-queue is empty or not
*/
static inline bool
_logger_queue_has() {
    return _logger_queue.size() > 0;
}


/*
*  the thread-handler
*/
static void *
_logger_queue_handler(void * args) {
    while (true)
    {
        if (!_logger_queue_has())
        {
            // set lock
            std::atomic_exchange(&_logger_lock_wait, true);
            _logger_queue_wait(30);
            continue;
        }
        // FIXIT: queue is not thread-safe container, if queue is writting when you read it...
        auto log = _logger_queue.front();
        _logger_queue_write(&log);
        _logger_queue.pop_front();
        // resize the queue
        if (_logger_queue.size() == 0)
            _logger_queue.shrink_to_fit();
    }
}


/*
*  start a logger-thread
*/
static void
_logger_queue_start()   {
    if (_logger_isstartup)
        return;

    // Create the Logger thread
    int ret = pthread_create(&_logger_thread, NULL, _logger_queue_handler, NULL);
    LABOR_ASSERT(ret == 0, "Create Logger thread error");

    _logger_isstartup = true;
}


/*
*  push a log into the logger-queue
*/
static void
_logger_queue_push(int level, const string & filename, int line, const string & content)    {
    _log_body_t ls;
    auto trueFile = _logger_queue_datefile();
    const char * filepath = _logger_strip_source_filename(filename.c_str());
    memcpy(ls.logpath, trueFile.c_str(), trueFile.length() + 1);
    memcpy(ls.filepath, filepath, strlen(filepath) + 1);
    memcpy(ls.text, content.c_str(), content.length() + 1);
    ls.line = line;
    ls.level = level;

    _logger_queue.push_back(ls);

    // if push success, resume the log thread immediately.
    _logger_queue_resume();
}


/* ------------------------------------
* The class implement
* ------------------------------------
*/
string labor::Logger::filepath_ = _load_config("log.file_path", "./");
string labor::Logger::format_ = _load_config("log.format", "$level>>$file|$line|$datetime| $text");
int labor::Logger::maxsize_ = _string2int(_load_config("log.file_size", "10"));
bool labor::Logger::enableStdout_ = _string2bool(_load_config("log.enable_stdout", "1"));


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
    _logger_queue_push((int)level_, source_, line_, __S(log_buffer));
}
