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
static unordered_map<string, zmq::socket_t> s_vm_pusher;

struct _lru_sockets
{
    string addr;
    zmq::socket_t * socket;

    _lru_sockets() : addr(""), socket(NULL) {}
};
static _lru_sockets s_lru_sockets[LABOR_MAX_PUSHER];
static int s_lru_cursor = 0;
static bool s_lru_full = false;

static void
_lru_sock_add(const char * addr)    {

}

static int
_lru_sock_find(const char * addr) {
    int now = s_lru_cursor, end = s_lru_full ? (s_lru_cursor+1) % LABOR_MAX_PUSHER : -1;
    do
    {
        if (now < 0) now = LABOR_MAX_PUSHER + now;
        auto & s = s_lru_sockets[now];
        if (s.addr.compare(addr) == 0)
            return now;
        now--;
    } while (now != end);
    return -1;
}


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
    return NULL;
}


void
labor::ext_service_publish(const char * message)    {
    if (s_vm_publisher == NULL)    {

    }    
    s_send(*s_vm_publisher, message);
}


void
labor::ext_service_push(const char * addr, const char * message)   {
    zmq::socket_t * pusher = NULL;
    if (s_vm_pusher.find(addr) == s_vm_pusher.end())
    {        
    }
    else
    {
        pusher = &s_vm_pusher[addr];
    }
    s_send(*pusher, message);
}


void
labor::ext_logger_debug(const char * message)   { LOG_DEBUG(message); }


void
labor::ext_logger_info(const char * message)    { LOG_INFO(message); }


void 
labor::ext_logger_warning(const char * message) { LOG_WARNING(message); }


void
labor::ext_logger_error(const char * message)   { LOG_ERROR(message); }