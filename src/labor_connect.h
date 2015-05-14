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
            PUSHPULL = 0,
        };
    public:
        Connector(ConnectorTypes type);
        ~Connector();

        bool send(const std::string & s) const;
        std::string recv() const;
        void bind(const std::string & addr) const;

    private:
        std::shared_ptr<_connector_impl> connector_;
        ConnectorTypes types_;
    };
}

#endif