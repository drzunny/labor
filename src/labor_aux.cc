#include "labor_aux.h"
#include "labor_log.h"
#include <unordered_map>

using namespace std;


/* -------------------------------------
*  Helper functions and properties
* -------------------------------------*/
static unordered_map<string, void *> s_service_pool;


/* -------------------------------------
 *  The Extension API definition
 * -------------------------------------*/
string
labor::ext_json_encode(const void * jsonptr)  {
    return "";
}


void
labor::ext_json_decode(const char * str, void ** jsonptr)   {

}


void
labor::ext_service_init(int type)   {

}


void
labor::ext_service_publish(void * hnd, const char * message)    {

}


void
labor::ext_service_push(void * hnd, const char * message)   {

}


void
labor::ext_service_pull(void * hnd)   {

}


void
labor::ext_logger_debug(const char * message)   { LOG_DEBUG(message); }


void
labor::ext_logger_info(const char * message)    { LOG_INFO(message); }


void 
labor::ext_logger_warning(const char * message) { LOG_WARNING(message); }


void
labor::ext_logger_error(const char * message)   { LOG_ERROR(message); }