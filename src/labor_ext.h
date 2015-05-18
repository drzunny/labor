#ifndef __LABOR_EXT_H__
#define __LABOR_EXT_H__

struct lua_State;

namespace labor
{
    class Extension
    {
    public:
        static void luaRegister(lua_State* vm);
        static void pyRegister();
    };
}

#endif