#include "labor_lvm.h"
#include "labor_log.h"

#include <lua.hpp>

using namespace std;
/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
bool
labor::LVM::init()  {
    return true;
}

void
labor::LVM::dispose()   {
    LOG_INFO("Lua VM was not initialized....");
}

int
labor::LVM::execute(const string & module, const string & args, labor::LVM::LVMType type)   {
    return 200;
}