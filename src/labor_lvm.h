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
        static void setupEnv(const std::vector<std::string> & paths);
        static bool init();
        static void dispose();

        void execute() const;

    private:
        static std::shared_ptr<LVM> s_pvm_;
    };
}

#endif