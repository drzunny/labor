#ifndef __LABOR_PYTHON_VM_H__
#define __LABOR_PYTHON_VM_H__

#include <string>
#include <memory>

namespace labor
{
    // The Python VM Manager for `labor`
    class PVM
    {
    public:
        enum PVMType
        {
            PUSHPULL = 0
        };

    public:
        static bool init();
        static void dispose();
        static void loadModule(const std::string & module, PVMType type = PVMType::PUSHPULL);

        static int execute(const std::string & module, const std::string & args = "", PVMType type = PVMType::PUSHPULL);
        static std::string lastError();

    private:
        static std::string lastError_;
    };
}

#endif
