#include "labor_request.h"
#include "labor_response.h"
#include "labor_def.h"

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
public:
    _request_impl(string && jsonMsg) :
        doc_(labor::JsonDoc::decode(jsonMsg)) {}


    void send() {
        // send request to service
    }

private:
    labor::JsonDoc doc_;
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
labor::Request::Request(string && jsonMsg)
    : request_(new _request_impl(std::move(jsonMsg)))
{
}


labor::Request::~Request()  {
    request_.reset();
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
    request_->send();
}