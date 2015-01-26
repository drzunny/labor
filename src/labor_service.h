#ifndef __LABOR_SERVICE_H__
#define __LABOR_SERVICE_H__

#include <string>
#include <memory>

namespace labor
{
    class PVM;
    class LVM;

    class Service
    {
    public:
        enum ServiceType
        {
            Python = 0,
            Lua = 1
        };

        static bool addService(const std::string & name, const std::string & path);
        static std::shared_ptr<Service> loadService(const std::string & name, bool *found = NULL);

    public:
        ~Service();

    private:
        Service(const std::string & name, ServiceType type, const std::string & path);

    };
}

#endif