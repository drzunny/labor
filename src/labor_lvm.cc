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
string labor::LVM::lastError_ = "";


bool
labor::LVM::init()  {
    return true;
}


void
labor::LVM::dispose()   {
    LOG_INFO("Lua VM was not initialized....");
}


void
labor::LVM::loadModule(const string & module, labor::LVM::LVMType type)    {

}


int
labor::LVM::execute(const string & module, const string & args, labor::LVM::LVMType type)   {
    return 200;
}


string
labor::LVM::lastError() {
    string msg = LVM::lastError_;
    LVM::lastError_ = "";
    return msg;
}