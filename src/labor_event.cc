#include "labor_event.h"
#include "labor_connect.h"
#include "labor_utils.h"

#include <stdlib.h>
#include <memory.h>
using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


struct _labor_msg_t
{
    char s_buff[64];
    char operation[32];
    int code;
    size_t l_len;
    char * l_buff;

    _labor_msg_t() : l_len(0), l_buff(NULL)
    {
        memset(s_buff, 0, 64);
        memset(operation, 0, 32);
    }

    ~_labor_msg_t()
    {
        FREE_IF_NOT_NULL(l_buff);
        l_len = 0;
    }

    void inline setbuf(const char * buf, size_t n)
    {
        if (n <= 64)
        {
            memcpy(s_buff, buf, n);
            FREE_IF_NOT_NULL(l_buff);
        }
        else
        {
            // realloc if n > l_len or n < 0.75 * l_len
            // so, if `n` in [0.75, 1] * l_len, re-use old buffer
            if (n < (size_t)(0.75 * l_len) || n > l_len)
            {
                free(l_buff);
                l_buff = (char*)realloc(l_buff, n);
            }
            memcpy(l_buff, buf, n);
        }
        l_len = n;
    }

    const inline char * buf()
    {
        return l_len <= 64 ? s_buff : l_buff;
    }

};

static unique_ptr<_labor_msg_t>
_msg_seperate(const string & msg, bool *ok)
{
    *ok = false;
    unique_ptr<_labor_msg_t> m(nullptr);
    if (msg.empty())
        goto _SEP_END;    
    const char * cstr = msg.c_str();
    if (cstr[0] != '\'')
        goto _SEP_END;
    

    
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

            bool p_ok = false, r_ok = false;
            _msg_seperate(p_msg, &p_ok);
            _msg_seperate(r_msg, &r_ok);
        }
    }

    void setEventHandler(const string & name, const string & resolve)
    {
    }

    void emit(const string & name)
    {
    }

private:
    labor::Connector pubsub_, reqrep_;

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