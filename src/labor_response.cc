#include "labor_response.h"
#include "labor_connect.h"
#include "labor_utils.h"
#include "labor_def.h"

#include <string>
#include <cassert>

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
labor::JsonDoc &&
_labor_make_response(const char * actionName, int code)   {
    LABOR_ASSERT(actionName != NULL, "ActionName cannot be NULL");
    labor::JsonDoc doc;
    doc.set("version", __S(LABOR_VERSION));
    doc.set("action", __S(actionName));

    return std::move(doc);
}

/* ------------------------------------
* The PIMPL Class
* ------------------------------------
*/
class labor::_response_impl
{
public:
    _response_impl(const string & action, int code) : body_(_labor_make_response(action.c_str(), code))
    {
    }
    ~_response_impl()   {
    }

    void setHeader(const string & name, const string & val) {
        LABOR_ASSERT(name.compare("data"), "Header name cannot be `data`");
        body_.set(name, const_cast<string&>(val));
    }

    void setBody(const string & body)   {
        body_.set(__S("data"), const_cast<string&>(body));
    }

    void send(const labor::Response::ConnectorRef & con) {
        auto && data = labor::JsonDoc::encode(body_);
        con->send(data);
    }
private:
    labor::JsonDoc body_;
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/

labor::Response::Response(const string & action, int code )
    : response_(new labor::_response_impl(action, code))
{
}


labor::Response::~Response()    {
    response_.reset();
}


void
labor::Response::setHeader(const string & name, const string & val)   {
    response_->setHeader(name, val);
}


void
labor::Response::send(const labor::Response::ConnectorRef & con) {
    response_->send(con);
}
