#include "labor_event.h"
#include "labor_connect.h"
#include "labor_request.h"
#include "labor_utils.h"
#include "labor_log.h"
#include "labor_def.h"

#include <stdlib.h>
#include <memory.h>
using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


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
                auto req = labor::Request(__S(p_msg));
                int code = req.send();
                switch (code)
                {
                case 404:
                    LOG_ERROR("service <%s> not found! ", req.actionName().c_str());
                    break;
                case 500:
                    LOG_ERROR("Service Error: %s", req.lastError().c_str());
                    break;

                default:
                    continue;
                }
            }
            else
            {
                LOG_ERROR("invalid msg: %s", p_msg.c_str());
            }
        }
    }

private:
    labor::Connector pubsub_;
    // labor::Connector reqrep_;

    void _init()
    {
        auto packages = labor::readInstallModules();
        auto pubsub_addr = labor::readConfig("labor.pubsub_addr");

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
    event_.reset();
}


void
labor::Event::run()
{
    this->event_->run();
}