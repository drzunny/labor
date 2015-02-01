#include "labor_request.h"
#include "labor_response.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


/* ------------------------------------
* The PIMPL Class
* ------------------------------------
*/
class labor::_request_impl
{
    
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
labor::Request::Request(const string & jsonMsg) : body_(labor::JsonDoc::decode(jsonMsg))
{
}


labor::Request::~Request()  {
}


bool
labor::Request::isValid(const string & msg)   {
    auto && json = labor::JsonDoc::decode(msg);
    // Check is null
    if (json.isNull())
        return false;
    // Check Require Elements
    if (json.has("action") && json.has("args") && json.has("header") &&
        json.isString("action"))
        return true;
    return false;
}


void
labor::Request::send()  {

}

shared_ptr<labor::Response>
labor::Request::waitForResponse()   {
    return nullptr;
}