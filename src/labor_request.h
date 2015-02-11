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
        Request(std::string && jsonMsg);
        ~Request();

        static bool isValid(const std::string & msg);

        std::string actionName();
        std::string lastError();
        int send();

    private:
        std::string lastError_;
        std::shared_ptr<_request_impl> request_;
    };
}


#endif