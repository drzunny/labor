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

        int serviceType() const;
        std::string actionName() const;
        std::string args() const;
        std::string lastError() const;
        int send();

    private:
        std::string lastError_;
        std::shared_ptr<_request_impl> request_;
    };
}


#endif
