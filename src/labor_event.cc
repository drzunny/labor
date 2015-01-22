#include "labor_event.h"
#include "labor_connect.h"
#include "labor_utils.h"

#include <vector>
#include <string>
using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
static vector<string>
_msg_seperate(const string & msg, bool *ok)
{
    *ok = false;
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
private:
    labor::Connector pubsub_, reqrep_;

    void _init()
    {
        auto packages = labor::readInstalledHandler();
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