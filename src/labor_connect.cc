#include "labor_connect.h"
#include <zmq.hpp>
#include <zhelpers.hpp>

using namespace std;

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
static int
_select_socket_type(labor::Connector::ConnectorTypes types)
{
    // if connector mode is pubsub
    if (types == labor::Connector::PUSHPULL)
        return ZMQ_PULL;
    else
        return ZMQ_REP;
}

zmq::context_t s_mq_context(1);

/* ------------------------------------
* The PIMPL
* ------------------------------------
*/
class labor::_connector_impl
{
public:
    _connector_impl(labor::Connector::ConnectorTypes types) :
        types_(types),
        socket_(zmq::socket_t(s_mq_context, _select_socket_type(types)))
    {
    }

    bool send(const string & s)
    {
        return s_send(socket_, s);
    }

    string recv()
    {
        return s_recv(socket_);
    }

    void bind(const string & addr)
    {
        if (this->types_ == labor::Connector::PUSHPULL)
            socket_.connect(addr.c_str());
    }

private:
    zmq::socket_t socket_;
    labor::Connector::ConnectorTypes types_;
};



/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
labor::Connector::Connector(labor::Connector::ConnectorTypes types) :
connector_(new _connector_impl(types)),
types_(types){ }


labor::Connector::~Connector()
{}


bool
labor::Connector::send(const string & s) const 
{
    return connector_->send(s);
}

string
labor::Connector::recv() const
{
    return connector_->recv();
}

void
labor::Connector::bind(const string & addr) const 
{
    connector_->bind(addr);
}
