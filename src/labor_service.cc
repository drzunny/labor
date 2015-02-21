#include "labor_service.h"
#include "labor_request.h"
#include "labor_pvm.h"
#include "labor_lvm.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
static inline int
_service_call_method(const char * method, const char * args, int type)    {
    // Check method is python or lua
    int methodType = 0;

    if (methodType == 0)
    {
        return labor::PVM::execute(method, args, (labor::PVM::PVMType)type);
    }
    else
    {
        return labor::LVM::execute(method, args, (labor::LVM::LVMType)type);
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
void
labor::Service::init()  {
}


bool
labor::Service::addHandler(const string & name, const string & script)    {
    return false;
}


int
labor::Service::handleRequest(const labor::Request * req, string & error) {
    int ret = _service_call_method(req->actionName().c_str(), req->args().c_str(), req->serviceType());
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