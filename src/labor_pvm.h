#ifndef __LABOR_PYTHON_VM_H__
#define __LABOR_PYTHON_VM_H__

#include <string>
#include <memory>
#include <Python.h>

namespace labor
{
    // The Python VM Manager for `labor`
    class PVM
    {
    public:
        enum PVMType
        {
            PUBSUB = 0,
            //REQREP = 1
        };

    public:
        static bool init();
        static void dispose();
        static void loadModule(const std::string & module, PVMType type = PVMType::PUBSUB);

        static int execute(const std::string & module, const std::string & args = "", PVMType type = PVMType::PUBSUB);
        static std::string lastError();

    private:
        static std::string lastError_;
    };
}

#endif