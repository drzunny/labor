#ifndef __LABOR_PYTHON_VM_H__
#define __LABOR_PYTHON_VM_H__

#include <vector>
#include <string>
#include <memory>
#include <Python.h>

namespace labor
{
    // The Python VM Manager for `labor`
    class PVM
    {
    public:
        static void setupEnv(const std::vector<std::string> & paths);
        static bool init();
        static void dispose();

        void execute() const;

    private:
        static std::shared_ptr<PVM> s_pvm_;

        PVM() {}
        ~PVM() {}
    };
}

#endif