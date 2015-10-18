#include "labor_aux.h"
#include "labor_log.h"
#include "labor_def.h"
#include "labor_utils.h"

#include <string.h>
#include <unordered_map>
#include <memory>

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
static zmq::context_t s_mq_context(1);
static zmq::socket_t * s_vm_publisher = NULL;
static unordered_map<string, weak_ptr<zmq::socket_t> > s_lru_reference;

struct _lru_sockets
{
    string addr;
    shared_ptr<zmq::socket_t> socket;

    _lru_sockets() : addr(""), socket(NULL) {}
};
static _lru_sockets s_lru_sockets[LABOR_MAX_PUSHER];
static int s_lru_cursor = 0;
static bool s_lru_full = false;

static shared_ptr<zmq::socket_t>
_lru_sock_add(const char * addr)    {
    shared_ptr<zmq::socket_t> s(NULL);

    if (s_lru_reference.find(addr) != s_lru_reference.end() && !s_lru_reference[addr].expired())
    {
        s = s_lru_reference[addr].lock();
    }
    else
    {
        s = shared_ptr<zmq::socket_t>(new zmq::socket_t(s_mq_context, ZMQ_PUSH));
        s_lru_reference[addr] = s;
    }
    s_lru_sockets[s_lru_cursor].addr = addr;
    s_lru_sockets[s_lru_cursor].socket = s;

    s_lru_cursor = (s_lru_cursor + 1) % LABOR_MAX_PUSHER;
    if (!s_lru_full && s_lru_cursor == LABOR_MAX_PUSHER - 1)
    {
        s_lru_full = true;
    }

    return s;
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


void
labor::ext_service_publish(const char * message)    {
    if (s_vm_publisher == NULL)    {
        s_vm_publisher = new zmq::socket_t(s_mq_context, ZMQ_PUB);
        s_vm_publisher->bind(labor::conf_read("labor.publish_addr").c_str());
    }
    s_send(*s_vm_publisher, message);
}


void
labor::ext_service_push(const char * addr, const char * message)   {
    auto socket = _lru_sock_add(addr);
    s_send(*(socket.get()), message);
}


void
labor::ext_logger_debug(const char * message)   { LOG_DEBUG(message); }


void
labor::ext_logger_info(const char * message)    { LOG_INFO(message); }


void
labor::ext_logger_warning(const char * message) { LOG_WARNING(message); }


void
labor::ext_logger_error(const char * message)   { LOG_ERROR(message); }
