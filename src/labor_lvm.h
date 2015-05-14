#ifndef __LABOR_LUAVM_H__
#define __LABOR_LUAVM_H__

#include <string>
#include <vector>
#include <memory>

namespace labor
{
    // The Lua VM Manager for `labor`
    class LVM
    {
    public:
        enum LVMType
        {
            PUSHPULL = 0
        };

    public:
        static bool init();
        static void dispose();
        static void loadModule(const std::string & module, LVMType type = LVMType::PUSHPULL);

        static int execute(const std::string & module, const std::string & args = "", LVMType type = LVMType::PUSHPULL);
        static std::string lastError();

    private:
        static std::string lastError_;
    };
}

#endif