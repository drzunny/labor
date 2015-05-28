#include "labor_ext.h"
#include "labor_aux.h"
#include "labor_utils.h"
#include "labor_log.h"

#include <string>
#include <Python.h>
#include <lua.hpp>
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

#define _LUA_SET_VALUE(i) if (i < 0) { lua_settable(L, -3);  } else { lua_rawseti(L, -2, idx);  }

// ----------------------------------
//  Helper functions
// ----------------------------------

//pre-declare it
static void s_lookup_lua_table(lua_State * L, Value * v, Document::AllocatorType& alloc);

static void
s_push_lua_value(lua_State * L, Value & v, int idx = -1)  {
    if (v.IsString()) { lua_pushstring(L, v.GetString()); _LUA_SET_VALUE(idx); }
    if (v.IsInt()) { lua_pushinteger(L, v.GetInt()); _LUA_SET_VALUE(idx); }
    if (v.IsInt64()) { lua_pushinteger(L, v.GetInt64()); _LUA_SET_VALUE(idx); }
    if (v.IsUint()) { lua_pushinteger(L, v.GetUint()); _LUA_SET_VALUE(idx); }
    if (v.IsUint64()) { lua_pushinteger(L, v.GetUint64()); _LUA_SET_VALUE(idx); }
    if (v.IsDouble()) { lua_pushnumber(L, v.GetDouble()); _LUA_SET_VALUE(idx); }
    if (v.IsBool()) { lua_pushboolean(L, v.GetBool() ? 1 : 0); _LUA_SET_VALUE(idx); }
    if (v.IsNull()) { lua_pushnil(L); _LUA_SET_VALUE(idx); }
}


static void
s_get_lua_value(lua_State * L, Value & v, Document::AllocatorType& alloc, const char * key = NULL) {
    int luaType = lua_type(L, -1);
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        Value child;
        s_lookup_lua_table(L, &child, alloc);
        if (key == NULL)
            v.PushBack(child, alloc);
        else
            v[key] = child;
    }
    else if (luaType == LUA_TBOOLEAN || luaType == LUA_TSTRING || luaType == LUA_TNUMBER || luaType == LUA_TNIL)
    {
        if (lua_isnumber(L, -1))
        {
            if (!key) v.PushBack(lua_tonumber(L, -1), alloc);
            else v[key] = lua_tonumber(L, -1);
        }
        else if (lua_isstring(L, -1))
        {
            if (!key) v.PushBack(StringRef(lua_tostring(L, -1)), alloc);
            else v[key] = StringRef(lua_tostring(L, -1));
        }
        else if (lua_isboolean(L, -1))
        {
            if (!key) v.PushBack(lua_toboolean(L, -1) == 1, alloc);
            else v[key] = lua_toboolean(L, -1) == 1;
        }
        else if (lua_isnil(L, -1))
        {
            if (!key) v.PushBack(Value(rapidjson::kNullType), alloc);
            else v[key].SetNull();
        }
    }
}


static void
s_lookup_lua_table(lua_State * L, Value * v, Document::AllocatorType& alloc)    {
    Value & d = *v;
    if (!lua_istable(L, -1))    {
        d.SetNull();
        return;
    }
    int len = lua_objlen(L, -1);
    if (len > 0)
    {
        d.SetArray();
        for (int i = 1; i <= len; i++)
        {
            lua_rawgeti(L, -1, i);
            s_get_lua_value(L, d, alloc);
            lua_pop(L, 1);
        }
    }
    else
    {
        d.SetObject();
        // push a nil for start lua_next
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
            if (lua_type(L, -2) != LUA_TSTRING) { lua_pop(L, 1); }
            const char * key = lua_tostring(L, -2);
            s_get_lua_value(L, d, alloc, key);
            lua_pop(L, 1);
        }
    }
}


static void
s_parse_lua_table(lua_State * L, Value * v)  {
    lua_newtable(L);
    Value & d = *v;
    if (d.IsArray())
    {
        for (auto i = 0u; i < d.Size(); i++)
        {
            // Lua's index is start from 1
            if (!d[i].IsArray() && !d[i].IsObject())   {
                s_push_lua_value(L, d[i], i + 1);
            }
            else
            {
                s_parse_lua_table(L, &d[i]);
                lua_rawseti(L, -2, i + 1);
            }
        }
    }
    else
    {
        for (auto iter = d.MemberBegin(); iter != d.MemberEnd(); iter++)
        {
            lua_pushstring(L, iter->name.GetString());
            if (!iter->value.IsArray() && !iter->value.IsObject())   {
                s_push_lua_value(L, iter->value);
            }
            else
            {
                s_parse_lua_table(L, &iter->value);
                lua_settable(L, -3);
            }
        }
    }
}



// ----------------------------------
//  Lua extension here
// ----------------------------------
static int
l_json_encode(lua_State * L) {
    /*
        only accept a table as parameter
        */
    if (!lua_istable(L, -1))
    {
        lua_pushstring(L, "only accept a table as parameter");
        lua_error(L);
        return 0;
    }
    Document d;
    s_lookup_lua_table(L, &d, d.GetAllocator());
    string json = labor::ext_json_encode((void*)&d);
    lua_pushstring(L, json.c_str());
    return 1;
}


static int
l_json_decode(lua_State * L) {
    const char * s = luaL_checkstring(L, -1);
    Document d;
    labor::ext_json_decode(s, (void*)&d);

    // The table will be set on the top of the stack after this function
    s_parse_lua_table(L, &d);
    return 1;
}


static int
l_service_push(lua_State * L) {
    const char * to = luaL_checkstring(L, -2);
    const char * msg = luaL_checkstring(L, -1);

    labor::ext_service_push(to, msg);
    return 0;
}


static int
l_service_publish(lua_State * L) {
    const char * msg = luaL_checkstring(L, -1);

    labor::ext_service_publish(msg);
    return 0;
}


static int
l_log_info(lua_State * L) {
    const char * msg = luaL_checkstring(L, -1);
    labor::ext_logger_info(msg);

    return 0;
}


static int
l_log_warning(lua_State * L) {
    const char * msg = luaL_checkstring(L, -1);
    labor::ext_logger_warning(msg);

    return 0;
}

static int
l_log_error(lua_State * L) {
    const char * msg = luaL_checkstring(L, -1);
    labor::ext_logger_error(msg);

    return 0;
}


static int
l_log_debug(lua_State * L) {
    const char * msg = luaL_checkstring(L, -1);
    labor::ext_logger_debug(msg);

    return 0;
}

// ----------------------------------
//  Python extension here
// ----------------------------------

static PyObject*
p_service_push(PyObject * self, PyObject * args) {
    const char *addr, *message;
    if (PyArg_ParseTuple(args, "ss", &addr, &message))
        labor::ext_service_push(addr, message);
    Py_RETURN_NONE;
}


static PyObject*
p_service_publish(PyObject * self, PyObject * args) {
    const char *message;
    if (PyArg_ParseTuple(args, "s", &message))
        labor::ext_service_publish(message);
    Py_RETURN_NONE;
}


static PyObject*
p_log_debug(PyObject * self, PyObject * args) {
    const char * str = NULL;
    if (!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }
    labor::ext_logger_debug(str);
    Py_RETURN_NONE;
}


static PyObject*
p_log_info(PyObject * self, PyObject * args) {
    const char * str = NULL;
    if (!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }
    labor::ext_logger_info(str);
    Py_RETURN_NONE;
}


static PyObject*
p_log_warning(PyObject * self, PyObject * args) {
    const char * str = NULL;
    if (!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }
    labor::ext_logger_warning(str);
    Py_RETURN_NONE;
}


static PyObject*
p_log_error(PyObject * self, PyObject * args) {
    const char * str = NULL;
    if (!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }
    labor::ext_logger_error(str);
    Py_RETURN_NONE;
}


// ----------------------------------
//  Register here
// ----------------------------------
static bool s_lua_initialize = false, s_py_initialize = false;

void
labor::Extension::luaRegister(lua_State * vm)   {
    if (s_lua_initialize)
        return;

    static luaL_Reg modules[] = {
        // Logger functions
        { "debug", l_log_debug },
        { "info", l_log_info },
        { "warning", l_log_warning },
        { "error", l_log_error },
        // Json functions
        { "jsonEncode", l_json_encode },
        { "jsonDecode", l_json_decode },
        // Service functions
        { "publish", l_service_publish },
        { "push", l_service_push },
        { NULL, NULL }
    };

    luaL_register(vm, "labor", modules);
    s_lua_initialize = true;
}


// Python has its own json parser, so we dont provide json api
static PyMethodDef modules[] = {
    // Logger functions
    { "debug", p_log_debug, METH_VARARGS, "use labor's debug logger" },
    { "info", p_log_info, METH_VARARGS, "use labor's info logger" },
    { "warning", p_log_warning, METH_VARARGS, "use labor's warning logger" },
    { "error", p_log_error, METH_VARARGS, "use labor's error logger" },
    // Service functions
    { "publish", p_service_publish, METH_VARARGS, "publish a message" },
    { "push", p_service_push, METH_VARARGS, "push a message" },
    { NULL, NULL, 0, NULL }
};


void
labor::Extension::pyRegister()  {
    if (s_py_initialize)
        return;

    if (Py_IsInitialized()) 
    {        
        Py_InitModule3("LABOR", modules, "Provide internal api for labor");
        s_py_initialize = true;
    }
    else
    {
        LOG_WARNING("Python extension disabled -- Python is not ready");
    }
}