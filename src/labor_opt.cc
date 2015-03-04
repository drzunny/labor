#include "labor_opt.h"
#include "labor_def.h"
#include "labor_utils.h"

#include <algorithm>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

using namespace std;

/*-----------------------
*   Options variants
*------------------------*/
static string                       s_labor_root = "";
static string                       s_option_conf = labor::path_getfull("./labor.conf");
static bool                         s_option_enablepy = true;
static bool                         s_option_enablelua = true;
static labor::Options::RunningMode  s_option_mode = labor::Options::Mode_Normal;

static bool s_option_has_help = false;
static bool s_option_has_version = false;

static const char * s_labor_help = "\
labor <options>\
[Options]\n \
\n\
--conf <file>               the path of `labor.conf`, default to `./labor.conf`\n\n\
--disabled <py|lua|none>    disable the service, default to none\n\n\
--mode <debug|normal|fast>  run in different mode, default to normal\n\n\
-h/--help                   show this help\n\n\
-v/--version                show version\n";


/*-----------------------
*   helper function
*------------------------*/

static inline bool
_options_is(const char * cur, const char * target)   {
    return strcmp(cur, target) == 0;
}


static inline bool
_options_in(const char * cur, int n, ...)   {
    va_list args;
    // because the params is next to `int n`
    va_start(args, n);
    for (auto i = 0; i < n; i++)    {
        const char * s = va_arg(args, const char*);
        if (_options_is(cur, s))
            return true;
    }
    va_end(args);
    return false;
}



/*-----------------------
*   Options implements
*------------------------*/

bool
labor::Options::parse(int argc, char * argv[])  {
    s_labor_root = argv[0];
    auto lastpos = s_labor_root.find_last_of("\\/");
    s_labor_root = s_labor_root.substr(0, lastpos);

    if (argc == 1)
        return true;
    
    // if i is a odd-number, it means it's a option name
    // else, position i of the argv is the value of position i-1
    // but when you meet -h or -v..... just return.
    const char * lastOpt = NULL;
    for (int i = 1; i < argc; i++)  {
        if (_options_in(argv[i], 4, "-h", "--help", "-v", "--version"))
        {
            // the priority of version is higher than help
            if (_options_in(argv[i], 2, "-v", "--version"))    {
                s_option_has_version = true;
            }   else    {
                s_option_has_help = true;
            }
            return true;
        }
        if (i % 2 != 0) 
        {
            // if odd-number is not a option
            if (!_options_in(argv[i], 3, "--conf", "--mode", "--disabled"))
                return false;
            lastOpt = argv[i];            
        }
        else
        {
            if (_options_is(lastOpt, "--conf"))
            {
                s_option_conf = labor::path_getfull(argv[i]);
            }
            else if (_options_is(lastOpt, "--mode"))
            {
                string runningMode(argv[i]);
                std::transform(runningMode.begin(), runningMode.end(), runningMode.begin(), ::tolower);
                if (!_options_in(runningMode.c_str(), 3, "normal", "debug", "fast"))
                    return false;
                if (_options_is(runningMode.c_str(), "normal"))
                    s_option_mode = Options::Mode_Normal;
                else if (_options_is(runningMode.c_str(), "debug"))
                    s_option_mode = Options::Mode_Debug;
                else
                    s_option_mode = Options::Mode_Optimized;
            }
            else if (_options_is(lastOpt, "--disabled"))
            {
                string disableLang(argv[i]);
                std::transform(disableLang.begin(), disableLang.end(), disableLang.begin(), ::tolower);
                if (_options_in(disableLang.c_str(), 2, "py", "python"))
                    s_option_enablepy = false;
                else if (_options_in(disableLang.c_str(), 2, "lua", "luajit"))
                    s_option_enablelua = false;
                else
                    return false;
            }
            lastOpt = NULL;
        }
    }
    return true;
}


const string &
labor::Options::configFile()    { return s_option_conf; }

bool 
labor::Options::enablePython()  { return s_option_enablepy; }

bool 
labor::Options::enableLua() { return s_option_enablelua; }

labor::Options::RunningMode
labor::Options::runningMode()   { return s_option_mode; }


bool 
labor::Options::checkAndShowHelp()  {
    if (s_option_has_help)  {
        fprintf(stdout, "%s\n\n", s_labor_help);
    }
    return s_option_has_help;
}


bool 
labor::Options::checkAndShowVersion()   {
    if (s_option_has_version)
    {
        fprintf(stdout, "labor version: %s\n\n", LABOR_VERSION);
    }
    return s_option_has_version;
}

const string &
labor::Options::laborRoot() {
    return s_labor_root;
}