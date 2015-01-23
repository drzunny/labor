#ifndef __LABOR_REQUEST_H__
#define __LABOR_REQUEST_H__

#include <string>
#include <memory>

namespace labor
{
    class Response;

    class Request
    {
    public:
        Request();
        ~Request();

        static bool isValid(const std::string & msg);

        void send();
        std::shared_ptr<Response> waitForResponse();

    private:
    };
}


#endif