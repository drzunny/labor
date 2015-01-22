#include "labor_utils.h"

#include <io.h>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace boost::property_tree;


/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/

// helper macros
#define _SET_IF_NOT_NULL(p, v) if (p != NULL) *##p = v


// Save the *.conf file's setting
static ptree s_conf_properties;
static string s_labor_conf;


static inline ptree &
_read_ini_config(const string & file, bool * ok = NULL)
{
    if (s_conf_properties.empty())
    {
        if (!labor::fileExists(file))
        {
            _SET_IF_NOT_NULL(ok, false);
            return s_conf_properties;
        }
        read_ini(file, s_conf_properties);
    }
    _SET_IF_NOT_NULL(ok, true);
    return s_conf_properties;
}

static vector<string>
_lookup_module_dirs(const string & modulePath)
{
    return vector<string>();
}


/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
void
labor::ready(int argc, char * argv[])
{
#ifndef LABOR_CONF
    s_labor_conf = "";
#else
    s_labor_conf = LABOR_CONF;
#endif
}


bool
labor::fileExists(const string & file)
{
#if WIN32
    return _access(file.c_str(), 0) == 0;
#else
    return access(file.c_str(), 0) == 0;
#endif
}


vector<string>
labor::readInstallModules()
{
    auto module_path = labor::readConfig("services.service_path");
    vector<string> modules = _lookup_module_dirs(module_path);

    return modules;
}


string
labor::readConfig(const string & name, const string & dval)
{
    bool ok = true;
    auto ini = _read_ini_config("", &ok);
    assert(ok == true);

    string v = ini.get<string>(name);
    if (v.empty()) return dval;
    return v;
}


vector<string>
labor::string_split(const string & s, char delm)
{    
    // FIXIT: this is not a efficient way to split
    istringstream iss(s);
    istringstream dm(delm);
    vector<string> out;

    std::copy(istream_iterator<string>(iss), istream_iterator<string>(dm), std::back_inserter(out));

    return out;
}