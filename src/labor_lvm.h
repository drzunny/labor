#ifndef __LABOR_LUAVM_H__
#define __LABOR_LUAVM_H__

#include <string>
#include <vector>
#include <memory>

namespace labor
{
    class LVM
    {
    public:
        enum LVMType
        {
            PUBSUB = 0,
            //REQREP = 1
        };

    public:
        static bool init();
        static void dispose();

        static void loadModule(const std::string & module, LVMType type);
        static int execute(const std::string & module, const std::string & args, LVMType type);

    private:
        static std::shared_ptr<LVM> s_pvm_;
    };
}

#endif