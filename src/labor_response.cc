#include "labor_response.h"
#include "labor_utils.h"
#include "labor_def.h"

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
void
_labor_make_response(const char * actionName, int code, const char * data)   {
    labor::JsonDoc doc;
    doc.set("version", LABOR_VERSION);
}

/* ------------------------------------
* The PIMPL Class
* ------------------------------------
*/
class labor::_response_impl
{
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/

labor::Response::Response() : response_(new labor::_response_impl())
{
}


labor::Response::~Response()    {
}


void
labor::Response::send() {

}