#include "labor_aux.h"
#include "labor_log.h"
#include "labor_def.h"

#include <string.h>
#include <unordered_map>

#include <zmq.hpp>
#include <zhelpers.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

using namespace std;
using namespace rapidjson;

/* -------------------------------------
*  Helper functions and properties
* -------------------------------------*/
static zmq::socket_t * s_vm_publisher = NULL;


/* -------------------------------------
 *  The Extension API definition
 * -------------------------------------*/
string
labor::ext_json_encode(void * jsonptr)  {
    Document & doc = *(Document*)jsonptr;
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}


void
labor::ext_json_decode(const char * str, void * jsonptr)   {
    LABOR_ASSERT(jsonptr != NULL, "Json output object cannot be null");
    Document & d = *(Document*)jsonptr;
    d.Parse(str);
}


void *
labor::ext_service_init_push(const char * addr)   {
    // TODO: create a push service in a LRU list;
}


void
labor::ext_service_publish(const char * message)    {
    if (s_vm_publisher == NULL)    {

    }    
    s_send(*s_vm_publisher, message);
}


void
labor::ext_service_push(void * hnd, const char * message)   {
    zmq::socket_t * s = (zmq::socket_t*)hnd;
    s_send(*s, message);
}


void
labor::ext_logger_debug(const char * message)   { LOG_DEBUG(message); }


void
labor::ext_logger_info(const char * message)    { LOG_INFO(message); }


void 
labor::ext_logger_warning(const char * message) { LOG_WARNING(message); }


void
labor::ext_logger_error(const char * message)   { LOG_ERROR(message); }