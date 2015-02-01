#ifndef __LABOR_REQUEST_H__
#define __LABOR_REQUEST_H__

#include <string>
#include <memory>
#include "labor_utils.h"

namespace labor
{
    class Response;
    class _request_impl;

    class Request
    {
    public:
        Request(const std::string & jsonMsg);
        ~Request();

        static bool isValid(const std::string & msg);

        void send();
        std::shared_ptr<Response> waitForResponse();

    private:
        std::shared_ptr<_request_impl> request_;
        JsonDoc body_;
    };
}


#endif