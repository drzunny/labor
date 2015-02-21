#include "labor_request.h"
#include "labor_service.h"
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


    string actionName() {
        return doc_.get("action").toString();
    }

    string args() {
        return doc_.get("args").toString();
    }

    int serviceType()   {
        return doc_.get("serviceType").toInt();
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

string
labor::Request::args() const    {
    return request_->args();
}


int
labor::Request::serviceType() const {
    return request_->serviceType();
}


string
labor::Request::actionName() const {
    return request_->actionName();
}


string
labor::Request::lastError() const {
    return lastError_;
}


int
labor::Request::send()  {
    return labor::Service::handleRequest(this, lastError_);
}