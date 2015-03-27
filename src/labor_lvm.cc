#include "labor_lvm.h"
#include "labor_log.h"
#include "labor_def.h"
#include "labor_utils.h"

#include <fstream>
#include <unordered_map>
#include <assert.h>

#include <lua.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

using namespace std;

#define __LVM_SET_TABLE_S(ls, k, v) \
    lua_pushstring((ls), (k));\
    lua_pushstring((ls), (v));\
    lua_settable((ls), -3)

#define __LVM_SET_TABLE_ET(ls, k) \
    lua_pushstring((ls), (k));\
    lua_newtable((ls));\
    lua_settable((ls), -3)

/* ------------------------------------
* The Module's variants
* ------------------------------------
*/
typedef lua_State* LuaVM;

static unordered_map<std::string, LuaVM> s_lua_vm = unordered_map< string, LuaVM >();


/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
static void
_lvm_json_to_table(lua_State * L, const char * json)    {
    using namespace rapidjson;
    Document doc;
    doc.Parse(json);
    if (doc.HasParseError())
    {
        LOG_ERROR("Invalid Json '%s'", json);
        return;
    }
    // read keys and set table. ignore Object and Array.
    size_t cnt = 0;
    lua_newtable(L);

    for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); iter++)
    {
        // check value
        if (iter->value.IsObject() || iter->value.IsArray())
            continue;

        // Push Key
        lua_pushstring(L, iter->name.GetString());        

        // Check and Push value
        if (iter->value.IsBool())
            lua_pushboolean(L, iter->value.GetBool() ? 1 : 0);
        else if (iter->value.IsInt())
            lua_pushinteger(L, iter->value.GetInt());
        else if (iter->value.IsUint())
            lua_pushinteger(L, iter->value.GetUint());
        else if (iter->value.IsInt64())
            lua_pushnumber(L, iter->value.GetInt64());
        else if (iter->value.IsUint64())
            lua_pushnumber(L, iter->value.GetUint64());
        else if (iter->value.IsDouble())
            lua_pushnumber(L, iter->value.GetDouble());
        else if (iter->value.IsNull())
            lua_pushnil(L);
        else if (iter->value.IsString())
            lua_pushstring(L, iter->value.GetString());
        else
            lua_pushnil(L);

        cnt += 1;
    }
    // set table in one time
    if (cnt > 0)
        lua_settable(L, -1 * (cnt * 2 + 1));
}


static void
_lvm_build_args(lua_State * L, const char * module, const char * args, const char * header, const char * ver)   {
    lua_newtable(L);

    // Push the request field into table
    __LVM_SET_TABLE_S(L, "module", module);
    __LVM_SET_TABLE_S(L, "version", ver == NULL ? "" : ver); 

    if (header == NULL)
    {
        __LVM_SET_TABLE_ET(L, "header");
    }
    else
    {
        lua_pushstring(L, "header");
        _lvm_json_to_table(L, header);
        lua_settable(L, -3);
    }

    if (args == NULL)
    {
        __LVM_SET_TABLE_ET(L, "args");
    }
    else
    {
        lua_pushstring(L, "args");
        _lvm_json_to_table(L, args);
        lua_settable(L, -3);
    }
    
}


static int
_lvm_error_traceback(lua_State * L)  {
    // Use Lua's `debug.trackback`
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1))    {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    // invoke debug.traceback
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}


static void
_lvm_package_path(LuaVM vm, string && working) {
    string luapath = "%work/?.lua;%work/?/init.lua;%work/lua/?.lua;%work/lua/?/init.lua";
#ifdef WIN32
    string cpath = "%work/?.dll;%work/?/init.dll;%work/lua/?.dll;%work/lua/?/init.dll";
#else
    string cpath = "%work/?.so;%work/?/init.so;%work/lua/?.so;%work/lua/?/init.so";
#endif
    labor::string_replace(luapath, "%work", working);
    labor::string_replace(cpath, "%work", working);

    // change current vm's `package.path`
    lua_getglobal(vm, "package");
    if (!lua_istable(vm, -1))
    {
        LOG_WARNING("`package` module not found in current Lua VM");
        lua_pop(vm, 1);
        return;
    }
    lua_pushstring(vm, luapath.c_str());
    lua_setfield(vm, -2, "path");

    // change current vm's `package.cpath`
    lua_pushstring(vm, cpath.c_str());
    lua_setfield(vm, -2, "cpath");

    // pop package
    lua_pop(vm, 1);
}


static inline bool
_lvm_create_service(const string & key, const string & luafile, string & msg) {
    LuaVM vm = luaL_newstate();
    luaL_openlibs(vm);

    // Change the lua vm package search path
    string working = luafile;
    // luafile = '$working/init.lua'
    labor::string_replace(working, "init.lua", "");
    _lvm_package_path(vm, std::move(working));

    // add `traceback` function in vm stack as the first element
    // to get all stack traceback, you must put traceback into stack,
    // use traceback function as callback for `lua_pcall`
    lua_pushcfunction(vm, _lvm_error_traceback);

    // equals to `luaL_dofile`, but handle the errors by myself
    luaL_loadfile(vm, luafile.c_str());
    int ret = lua_pcall(vm, 0, LUA_MULTRET, 1);

    if (ret != 0)
    {
        msg.append(lua_tostring(vm, -1));
        msg.append("\n load Lua service error");
        return false;
    }

    // save vm
    s_lua_vm[key] = vm;
    return true;
}


static int
_lvm_service_execute(const string & module, const string & args, labor::LVM::LVMType type, string & msg)    {
    string key(module);
    key.append(type == labor::LVM::PUBSUB ? "_0" : "_1");

    if (s_lua_vm.find(key) == s_lua_vm.end())
    {
        msg = "module `$name` with `$mode` is not found";
        labor::string_replace(msg, "$name", module);
        labor::string_replace(msg, "$mode", type == labor::LVM::PUBSUB ? "PUBSUB" : "REQREP");
        return 404;
    }

    LuaVM & vm = s_lua_vm[key];

    // get function
    lua_getglobal(vm, "subscript");
    if (!lua_isfunction(vm, -1))
    {
        lua_pop(vm, 1);
        msg = "`subscript` not found in service";
        return 500;
    }

    // build arguments
    _lvm_build_args(vm, module.c_str(), args.c_str(), NULL, NULL);

    // execute, `errfunc` is call back element,
    int ret = lua_pcall(vm, 1, 0, 1);
    if (ret != 0)
    {
        // lua's return will be pop here
        msg.append(lua_tostring(vm, -1));
        lua_pop(vm, 1);
        switch (ret)
        {
        case LUA_ERRRUN:
            msg.append("\nRuntime error!\n "); break;
        case LUA_ERRMEM:
            msg.append("\nmemory allocation error\n"); break;
        case LUA_ERRERR:
            msg.append("\nerror while running the error handler function\n"); break;
        default:
            msg.append("\nUnknown\n"); break;
        }       

        return 500;
    }
    

    return 0;
}

/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
string labor::LVM::lastError_ = "";


bool
labor::LVM::init()  {
    // do nothing in LVM::init
    return true;
}


void
labor::LVM::dispose()   {
    if (s_lua_vm.size() == 0)   {
        LOG_INFO("Lua VM was not initialized....");
        return;
    }
    auto iter = s_lua_vm.begin();
    while (iter != s_lua_vm.end())
    {
        lua_close(iter->second);
    }
    s_lua_vm.clear();
    s_lua_vm.swap(decltype(s_lua_vm)());
}


void
labor::LVM::loadModule(const string & module, labor::LVM::LVMType type)    {
    string moduleKey(module);
    moduleKey.append(type == labor::LVM::PUBSUB ? "_0" : "_1");
    if (s_lua_vm.find(moduleKey) != s_lua_vm.end())
    {
        // TODO: Check MD5 for upgrade it
        return;
    }
    LABOR_ASSERT(!module.empty(), "module name cannot be empty");
    // because current cwd is services dir
    string modulePath = "./$name/init.lua";
    labor::string_replace(modulePath, "$name", module);

    // different lua_State per service
    string loadMsg;
    if (!_lvm_create_service(moduleKey, modulePath, loadMsg))
    {
        LOG_ERROR("load service `%s` error:\n%s", module.c_str(), loadMsg.c_str());
    }
}


int
labor::LVM::execute(const string & module, const string & args, labor::LVM::LVMType type)   {
    string msg;
    int code = _lvm_service_execute(module, args, type, msg);
    if (code != 0)    {
        if (code == 500)
        {
            LVM::lastError_ = msg;
            LOG_ERROR("Lua Service <%s> Error(500)", module.c_str());
        }
        else
        {
            LOG_ERROR("Call Lua Service <%s> - fail(%d) : %s", module.c_str(), code, msg.c_str());
        }
    }
    return code;
}


string
labor::LVM::lastError() {
    string msg = LVM::lastError_;
    LVM::lastError_ = "";
    return msg;
}