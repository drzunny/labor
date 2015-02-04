#include "labor_opt.h"
#include "labor_def.h"

#include <stdio.h>
#include <string.h>

using namespace std;

/*-----------------------
*   Options variants
*------------------------*/
static string                       s_option_conf = "./labor.conf";
static bool                         s_option_enablepy = true;
static bool                         s_option_enablelua = true;
static labor::Options::RunningMode  s_option_mode = labor::Options::Mode_Normal;

static bool s_option_has_help = false;
static bool s_option_has_version = false;

static const char * s_labor_help = "\
labor <options>\
[Options]\n \
\n\
--conf <file>\t\tthe path of `labor.conf`, default to `./labor.conf`\n\
--disabled <python|lua|none>\t\tdisable the service of python or lua, default to none(enable py and lua)\n\
--mode <debug|normal|optimize>\t\t enable labor run in different mode, default to normal\n\
-h/--help\t\tshow this help\n\
-v/--version\t\tshow version\n";


/*-----------------------
*   helper function
*------------------------*/

static inline bool
_options_is(const char * cur, const char * target)   {
    return strcmp(cur, target) == 0;
}


static inline bool
_options_in(const char * cur, ...)   {

}



/*-----------------------
*   Options implements
*------------------------*/

bool
labor::Options::parse(int argc, char * argv[])  {
    if (argc == 1)
        return true;

    // if i is a odd-number, it means it's a option name
    // else, position i of the argv is the value of position i-1
    // but when you meet -h or -v..... just return.
    for (int i = 0; i < argc; i++)  {
        if (_options_in(argv[i], "-h", "--help", "-v", "--version"))
        {
            // the priority of version is higher than help
            if (_options_in(argv[i], "-v", "--version"))    {
                s_option_has_version = true;
            }   else    {
                s_option_has_help = true;
            }
            return true;
        }
        if (i % 2 != 0) 
        {
            // if odd-number is not a option
            if (!_options_in(argv[i], "--conf", "--mode", "--disabled"))
                return false;
        }
        else
        {

        }
    }
}


const string &
labor::Options::ConfigFile()    { return s_option_conf; }

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