#ifndef __LABOR_RESPONSE_H__
#define __LABOR_RESPONSE_H__

#include <string>
#include <memory>

namespace labor
{
    class _response_impl;
    class Connector;

    class Response
    {
    public:
        typedef std::shared_ptr < Connector > ConnectorRef;
    public:
        Response(const std::string & action, int code);
        ~Response();

        void setHeader(const std::string & name, const std::string & val);
        void setBody(const std::string & body);
        void send(const ConnectorRef & con);

    private:
        std::shared_ptr<_response_impl> response_;
    };
}


#endif
