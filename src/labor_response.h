#ifndef __LABOR_RESPONSE_H__
#define __LABOR_RESPONSE_H__

#include <memory>

namespace labor
{
    class _response_impl;

    class Response
    {
    public:
        Response();
        ~Response();

        void send();

    private:
        std::shared_ptr<_response_impl> response_;
    };
}


#endif