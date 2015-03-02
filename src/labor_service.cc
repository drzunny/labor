#include "labor_service.h"
#include "labor_request.h"
#include "labor_pvm.h"
#include "labor_lvm.h"
#include "labor_log.h"
#include "labor_def.h"
#include "labor_opt.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
static string _service_root_dir = labor::conf_read("services.service_path", "./services");


static inline int
_service_call_method(const char * method, const char * args, int type, string & err)    {
    // Check method is python or lua
    int methodType = 0;
    int ret = -1;

    if (methodType == 0)    
    {
        ret = labor::PVM::execute(method, args, (labor::PVM::PVMType)type);
        if (ret == 500)
            err = labor::PVM::lastError();
    }
    else    
    {
        ret = labor::LVM::execute(method, args, (labor::LVM::LVMType)type);
        if (ret == 500)
            err = labor::LVM::lastError();
    }
    return ret;
}


static inline int
_service_check_lang(const char * module) {
    // because the `cwd` has been changed. no necessary for abspath
    string pythonFile = "./$name/__init__.py";
    string luaFile = "./$name/init.lua";
    labor::string_replace(pythonFile, "$name", __S(module));
    labor::string_replace(luaFile, "$name", __S(module));

    // Python first
    if (labor::Options::enablePython() && labor::path_exists(pythonFile))
    {
        return 0;
    }
    if (labor::Options::enableLua() && labor::path_exists(luaFile))
    {
        return 1;
    }

    return -1;
}

/* ------------------------------------
* The PIMPL Implementations
* ------------------------------------
*/
class labor::_service_impl
{
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
bool
labor::Service::setEnv()    {
    string fullpath = labor::path_getfull(_service_root_dir);
    labor::path_chdir(fullpath);
    return true;
}


void
labor::Service::init()  {
}


bool
labor::Service::addHandler(const string & name)    {
    int lang = _service_check_lang(name.c_str());
    if (lang < 0)
    {
        LOG_ERROR("module `%s` cannot be resolved.", name.c_str());
        return false;
    }
    switch (lang)
    {
    case (int)labor::Service::Python:
        labor::PVM::loadModule(name, labor::PVM::PUBSUB);
        //labor::PVM::loadModule(name, labor::PVM::REQREP);
        return true;
    case (int)labor::Service::Lua:
        labor::LVM::loadModule(name, labor::LVM::PUBSUB);
        //labor::LVM::loadModule(name, labor::LVM::REQREP);
        return true;
    default:
        LOG_ERROR("unknown lang?");
        return false;
    }
}


int
labor::Service::handleRequest(const labor::Request * req, string & error) {
    int ret = _service_call_method(req->actionName().c_str(), req->args().c_str(), req->serviceType(), error);
    return ret;
}



labor::Service::Service(labor::Service::ServiceType type, const string & action, const string & handlePath)   {
}


labor::Service::~Service()   {
}


void
labor::Service::publish(const labor::JsonDoc & args)    {
}


#if defined(LABOR_IS_READY)
void
labor::Service::requestTo(const labor::JsonDoc & argc, int timeout) {
}
#endif