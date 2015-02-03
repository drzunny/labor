#ifndef __LABOR_RESPONSE_H__
#define __LABOR_RESPONSE_H__

#include <string>
#include <memory>

namespace labor
{
    class _response_impl;

    class Response
    {
    public:
        Response(const std::string & action, int code);
        ~Response();

        void setHeader(const std::string & name, const std::string & val);        
        void send(const std::string & body);

    private:
        std::shared_ptr<_response_impl> response_;
    };
}


#endif