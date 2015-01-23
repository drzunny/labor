#include "labor_event.h"
#include "labor_connect.h"
#include "labor_request.h"
#include "labor_response.h"
#include "labor_utils.h"

#include <stdlib.h>
#include <memory.h>
#include <unordered_map>
using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
#define _MSG_SP_DELM '\27'
#define Hashtable unordered_map<string, string>


struct _labor_msg_t
{
    char s_buff[64];
    char operation[32];
    size_t l_len;
    char * l_buff;

    _labor_msg_t() : l_len(0), l_buff(NULL) {
        memset(s_buff, 0, 64);
        memset(operation, 0, 32);
    }

    ~_labor_msg_t() {
        FREE_IF_NOT_NULL(l_buff);
        l_len = 0;
    }

    void setop(const char * op, size_t n)   {
        if (n > 32) {
            memcpy(operation, op, 32);
        }   else    {
            memcpy(operation, op, n);
            memset(operation + n, 0, 32 - n);
        }
    }

    void inline setbuf(const char * buf, size_t n)  {
        if (n <= 64)
        {
            memcpy(s_buff, buf, n);
            FREE_IF_NOT_NULL(l_buff);
        }
        else
        {
            if (n < (size_t)(0.75 * l_len) || n > l_len)
            {
                free(l_buff);
                l_buff = (char*)realloc(l_buff, n);
            }
            memcpy(l_buff, buf, n);
        }
        l_len = n;
    }

    const inline char * buf()   {
        return l_len <= 64 ? s_buff : l_buff;
    }

};


static unique_ptr<_labor_msg_t>
_msg_seperate(const string & msg, bool *ok)
{
    *ok = false;
    unique_ptr<_labor_msg_t> m(nullptr);
    auto s = vector<string>();
    if (msg.empty())
        goto _SEP_END;    
    const char * cstr = msg.c_str();
    if (cstr[0] != '\'')
        goto _SEP_END;

    s = labor::string_split(msg, _MSG_SP_DELM);
    if (s.size() != 2)
        goto _SEP_END;

    m = unique_ptr<_labor_msg_t>(new _labor_msg_t());
    m->setop(s[0].c_str(), s[0].length());
    m->setbuf(s[1].c_str(), s[1].length());
    *ok = true;
    
_SEP_END:
    return m;
}

/* ------------------------------------
* The PIMPL
* ------------------------------------
*/
class labor::_event_impl
{
public:
    _event_impl() :
        pubsub_(labor::Connector(labor::Connector::PUBSUB)),
        reqrep_(labor::Connector(labor::Connector::REQREP)) {}

    void run()
    {
        this->_init();

        while (true)
        {
            auto p_msg = pubsub_.recv();
            auto r_msg = reqrep_.recv();

            if (labor::Request::isValid(p_msg))
            {
                continue;
            }
            if (labor::Request::isValid(r_msg))
            {
                continue;
            }
        }
    }

    void setEventHandler(const string & name, const string & resolve)
    {
        if (eventHandlers_.find(name) == eventHandlers_.end())
            eventHandlers_[name] = resolve;
    }

    void emit(const string & name)
    {
        if (eventHandlers_.find(name) == eventHandlers_.end())  {
            // TODO: Try to find, if not found, log it
            return;
        }
        // TODO: send a request to handlers
    }

private:
    labor::Connector pubsub_, reqrep_;
    Hashtable eventHandlers_;

    void _init()
    {
        auto packages = labor::readInstallModules();
        auto pubsub_addr = labor::readConfig("labor", "pubsub_addr");
        auto reqrep_addr = labor::readConfig("labor", "reqrep_addr");

        for (auto p : packages)
        {
            pubsub_.setFilter(p);
            reqrep_.setFilter(p);
        }
    }
};

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
labor::Event::Event()
    : event_(new labor::_event_impl())
{
}


labor::Event::~Event()
{
}


void
labor::Event::setEventHandler(const string & name, const string & resolve)
{
    event_->setEventHandler(name, resolve);
}


void
labor::Event::emit(const string & name)
{
    event_->emit(name);
}

void
labor::Event::run()
{
    this->event_->run();
}