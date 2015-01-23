#ifndef __LABOR_CONNECT_H__
#define __LABOR_CONNECT_H__

#include <memory>
#include <string>
#include <stdint.h>

namespace labor
{
    class _connector_impl;
    class Connector
    {
    public:
        enum ConnectorTypes
        {
            PUBSUB = 0,
            REQREP = 1
        };
    public:
        Connector(ConnectorTypes type);
        ~Connector();

        bool send(const std::string & s) const;
        std::string recv() const;
        void bind(const std::string & addr) const;
        void setFilter(const std::string & name);

    private:
        std::shared_ptr<_connector_impl> connector_;
        ConnectorTypes types_;
    };
}

#endif