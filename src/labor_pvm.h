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
        PVM();

        void daemon() const;
        void execute() const;

    private:
        std::unique_ptr<int> vm_;        
    };
}

#endif