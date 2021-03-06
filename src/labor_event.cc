#include "labor_event.h"
#include "labor_connect.h"
#include "labor_request.h"
#include "labor_service.h"
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
        pushpull_(labor::Connector(labor::Connector::PUSHPULL)) {}

    void run()
    {
        this->_init();

        while (true)
        {
            auto p_msg = pushpull_.recv();
            if (labor::Request::isValid(p_msg))
            {
                auto req = labor::Request(__S(p_msg));
                int code = req.send();
                switch (code)
                {
                case 404:
                    LOG_ERROR("Service <%s> not found! ", req.actionName().c_str());
                    break;
                case 500:
                    LOG_ERROR("Service <%s> Error\n%s", req.actionName().c_str(), req.lastError().c_str());
                    break;
                case -1:
                    LOG_ERROR("Unknown Error");
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
    labor::Connector pushpull_;


    inline string _readPubsubAddrs() {
        return labor::conf_read("labor.address");
    }


    void _init()
    {
        auto packages = labor::conf_modules();
        auto pushpull_addrs = string("tcp://") + this->_readPubsubAddrs();

        if (packages.size() == 0)   {
            LOG_WARNING("no services has been loaded....");
        }

        // you can bind multi-addr
        pushpull_.bind(pushpull_addrs);
        LOG_INFO("bind service: %s", pushpull_addrs.c_str());

        for (auto p : packages)
        {
            labor::Service::addHandler(p);
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
