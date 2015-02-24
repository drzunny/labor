#ifndef __LABOR_SERVICE_H__
#define __LABOR_SERVICE_H__

#include "labor_utils.h"

#include <string>
#include <memory>

namespace labor
{
    class _service_impl;
    class Request;

    class Service
    {
        /*
        * As a Module
        */
    public:
        enum ServiceType
        {
            Python = 0,
            Lua = 1
        };

        static bool setEnv();
        static void init();
        static bool addHandler(const std::string & name);
        static int handleRequest(const Request * req, std::string & error = std::string(""));
        static std::string lastError();


        /*
        * As a Instance
        */
    public:
        ~Service();

        void publish(const JsonDoc & args);

        /*
            These methods are not ready not. But they will be implemented at next version.
        */
#if defined(LABOR_IS_READY)
        void requestTo(const JsonDoc & args, int timeout = 3000);
#endif

    private:
        std::shared_ptr<_service_impl> service_;

        Service(ServiceType type, const std::string & action, const std::string & handlePath);
    };
}

#endif