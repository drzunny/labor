#include "labor_event.h"
#include "labor_connect.h"
#include "labor_utils.h"

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
        pubsub_(labor::Connector(labor::Connector::PUBSUB)),
        reqrep_(labor::Connector(labor::Connector::REQREP)) {}

    void run()
    {
        this->_init();

        while (true) 
        {
        }
    }
private:
    labor::Connector pubsub_, reqrep_;

    void _init()
    {
        auto packages = labor::readInstalledHandler();
        auto pubsub_addr = labor::readConfig("pubsub", "address");
        auto reqrep_addr = labor::readConfig("reqrep", "address");

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