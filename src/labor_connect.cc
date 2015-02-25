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
    if (types == labor::Connector::PUBSUB)
        return ZMQ_SUB;
    else
        return ZMQ_REP;
}

/* ------------------------------------
* The PIMPL
* ------------------------------------
*/
class labor::_connector_impl
{
public:
    _connector_impl(labor::Connector::ConnectorTypes types) :
        ctx_(zmq::context_t(1)),
        types_(types),
        socket_(zmq::socket_t(ctx_, _select_socket_type(types)))
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
        if (this->types_ == labor::Connector::PUBSUB)
            socket_.connect(addr.c_str());
    }

    void setFilter(const string & name)
    {
        if (types_ == labor::Connector::PUBSUB)
            socket_.setsockopt(ZMQ_SUBSCRIBE, name.c_str(), name.length());
    }

private:
    zmq::context_t ctx_;
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


void
labor::Connector::setFilter(const string & name)
{
    connector_->setFilter(name);
}


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
