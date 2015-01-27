#include "labor_event.h"
#include "labor_connect.h"
#include "labor_request.h"
#include "labor_response.h"
#include "labor_utils.h"
#include "labor_log.h"

#include <stdlib.h>
#include <memory.h>
#include <unordered_map>
using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
#define _MSG_SP_DELM "\27"
#define Hashtable unordered_map<string, string>


/* ------------------------------------
* The PIMPL
* ------------------------------------
*/

class labor::_event_impl
{
public:
    _event_impl() :
        pubsub_(labor::Connector(labor::Connector::PUBSUB)) {}

    void run()
    {
        this->_init();

        while (true)
        {
            auto p_msg = pubsub_.recv();
            if (labor::Request::isValid(p_msg))
            {
                continue;
            }
            else
            {
                LOG_INIT();
                LOG_ERROR("invalid msg: %s", p_msg.c_str());
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
    labor::Connector pubsub_;
    // labor::Connector reqrep_;
    Hashtable eventHandlers_;

    void _init()
    {
        auto packages = labor::readInstallModules();
        auto pubsub_addr = labor::readConfig("labor", "pubsub_addr");

        for (auto p : packages)
        {
            pubsub_.setFilter(p);
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