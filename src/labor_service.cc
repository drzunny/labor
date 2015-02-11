#include "labor_service.h"
#include "labor_request.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


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
    return 200;
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