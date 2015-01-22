#include "labor_utils.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/

// Save the *.conf file's setting
static int s_conf_properties = 0;

static int
_read_ini_config(const string & file)   
{
    if (!s_conf_properties)
    {

    }
    return s_conf_properties;
}


/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
vector<string>
labor::readInstalledHandler()
{
}

string
labor::readConfig(const string & group, const string & name, const string & dval)  
{
}