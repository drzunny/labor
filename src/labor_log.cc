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
#include <algorithm>
#include <boost/algorithm/string.hpp>



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
* The CAS(?) Queue
* ------------------------------------
*/
#define __ATOM_QGET(x)   atomic_load<int>(&(x))
#define __ATOM_QSET(x,v) atomic_exchange<int>(&(x), (v))
#define __ATOM_QGETV(x)  cas_queue[__ATOM_QGET(x)].get()
#define __ATOM_QADD(x,v) atomic_fetch_add<int>(&(x), (v))
#define _CAS_RING_SIZE   128

struct _log_body_t;

static shared_ptr<_log_body_t> cas_queue[_CAS_RING_SIZE];
static atomic<int> s_cas_queue_head;
static atomic<int> s_cas_queue_tail;


static inline void
s_cas_queue_init()  {
    memset(cas_queue, 0, _CAS_RING_SIZE);
}

static inline bool
s_cas_queue_empty()    {
    return __ATOM_QGET(s_cas_queue_head) == __ATOM_QGET(s_cas_queue_tail);
}


static void
s_cas_queue_push(_log_body_t * data)   {
    auto sptr = shared_ptr<_log_body_t>(data);
    cas_queue[__ATOM_QGET(s_cas_queue_tail)] = sptr;
    auto lastpos = __ATOM_QADD(s_cas_queue_tail, 1);
    if (lastpos == _CAS_RING_SIZE - 1)
    {
        __ATOM_QSET(s_cas_queue_tail, 0);
    }
}

static shared_ptr<_log_body_t>
s_cas_queue_pop() {
    if (s_cas_queue_empty())
        return shared_ptr<_log_body_t>();

    auto ret = cas_queue[__ATOM_QGET(s_cas_queue_head)];
    auto lastpos = __ATOM_QADD(s_cas_queue_head, 1);
    if (lastpos == _CAS_RING_SIZE - 1)
    {
        __ATOM_QSET(s_cas_queue_head, 0);
    }

    return ret;
}


/* ------------------------------------
* The helpers
* ------------------------------------
*/
static int
_string2int(string && s)   {
    if (s.empty())
        return 0;
    return std::atoi(s.c_str());
}


static bool
_string2bool(string && s) {
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    if (s.compare("1") == 0 || s.compare("true") == 0 || s.compare("yes") == 0)
        return true;
    return false;
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
        return "[DEBUG]";
    case labor::Logger::LV_WARNING:
        return "[WARNING]";
    case labor::Logger::LV_ERROR:
        return "[ERROR]";
    default:
        return "[INFO] ";
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

/* -------------------------------------------------------------------
*   logger operation implements
*  -------------------------------------------------------------------
*/

struct _log_body_t {
    string text;
    string logpath;
    string filepath;
    int line;
    int level;
};


static bool _logger_isstartup = false;
static pthread_t _logger_thread;

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
        if (!_logger_file_exists(log->logpath.c_str()))
            s_logfile_handle = fopen(log->logpath.c_str(), "w");
        else
            s_logfile_handle = fopen(log->logpath.c_str(), "a");

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

#ifdef LABOR_DEBUG
    if (check && labor::Logger::enableStdout())
        printf("\n[INFO]  the queue was been unlocked\n");
#endif
}


/*
*  a spinlock and timer for waiting the log arrived
*/
static void
_logger_queue_wait(size_t secs)   {
    static uint64_t ts = labor::timestamp_now();

    // To avoid spinlock's idle-loop to waste the CPU. use sleep(1)
    while (std::atomic_load(&_logger_lock_wait) == true)
    {
        labor::time_sleep(1);
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
    return !s_cas_queue_empty();
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
            _logger_queue_wait(10);
            continue;
        }
        auto log = s_cas_queue_pop();
        _logger_queue_write(log.get());
        //labor::time_sleep(1);
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
    _log_body_t * ls = new _log_body_t;
    auto trueFile = _logger_queue_datefile();
    const char * filepath = _logger_strip_source_filename(filename.c_str());
    ls->logpath = trueFile;
    ls->filepath = filepath;
    ls->text = content;
    ls->line = line;
    ls->level = level;

    s_cas_queue_push(ls);

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


bool
labor::Logger::init()  {
    s_cas_queue_init();
    _logger_queue_start();
    return true;
}


void
labor::Logger::dispose()    {
    while (!s_cas_queue_empty())
        s_cas_queue_pop();
}


labor::Logger::Logger(labor::Logger::LoggerLevel level, const char * filename, int line)
    : level_(level), source_(filename), line_(line) {
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
