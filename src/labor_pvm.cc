#include "labor_pvm.h"
#include "labor_log.h"
#include "labor_utils.h"
#include "labor_def.h"

#include <unordered_map>

#include <Python.h>

using namespace std;
/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/

struct _pvm_module_t
{
    int argc;
    PyObject * module;
    PyObject * method;
};


static const char * _pvm_script_change_cwd = "import os, sys\n__pvm_cwd = os.path.abspath('$file')\nsys.path[0] = __pvm_cwd\nos.chdir(__pvm_cwd)";
static string _pvm_service_root = labor::readConfig("services.service_path", "./services");
static Hashtable<string, _pvm_module_t> s_pvm_module = Hashtable<string, _pvm_module_t>();


/* ------------------------------------
* The PVM Helpers
* ------------------------------------
*/

static bool
_pvm_helper_chdir() {
    // check directory exists
    if (!labor::fileExists(_pvm_service_root))   {
        LOG_ERROR("Service directory <%s> not found!", _pvm_service_root.c_str());
        return false;
    }
    string script(_pvm_script_change_cwd);
    labor::string_replace(script, "$file", _pvm_service_root);
    return PyRun_SimpleString(script.c_str()) == 0;
}


static int
_pvm_service_exec(const string & module, const labor::JsonDoc & args, string & msg)    {
    if (s_pvm_module.find(module) == s_pvm_module.end())
    {
        msg = "not found";
        return 404;
    }
    auto & m = s_pvm_module[module];
    // method format
    PyObject * vars = PyList_New(args.count());
    for (size_t i = 0; i < args.count(); i++)
    {
        auto a = Py_BuildValue("u", args.getIndex(i).toString().c_str());
        PyList_Append(vars, a);
        Py_DECREF(a);
    }

    PyObject_Call(m.method, vars, NULL);
    Py_DECREF(vars);

    return 0;
}



/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
bool
labor::PVM::init()  {
    // Init the Python VM. Fuck GIL.
    if (!Py_IsInitialized())    {
        Py_SetProgramName("labor");
        Py_Initialize();
        // change the cwd
        if (!_pvm_helper_chdir())   {
            LOG_ERROR("init services fail");
            return false;
        }
        labor::PVM::loadModule("echo");
    }

    return true;
}

void
labor::PVM::dispose()   {
    if (Py_IsInitialized())
    {
        Py_Finalize();
    }
    else
    {
        LOG_INFO("Python was not initialized....");
    }
}

void
labor::PVM::loadModule(const string & module, labor::PVM::PVMType type)  {
    if (s_pvm_module.find(module) != s_pvm_module.end())
    {
        // TODO: Check MD5 for upgrade it
        return;
    }
    LABOR_ASSERT(!module.empty(), "module name cannot be empty");
    // because current cwd is services dir
    string modulePath = "./$name/__init__.py";
    labor::string_replace(modulePath, "$name", module);

    if (!labor::fileExists(modulePath)) {
        LOG_ERROR("module <%s> not found.", module.c_str());
        return;
    }

    // load from module and cachne it
    PyObject * pymodule = NULL, *method = NULL;
    pymodule = PyImport_ImportModule(module.c_str());
    if (!pymodule)    {
        LOG_INFO("Load module %s fail", module.c_str());
        return;
    }
    if (type == labor::PVM::PVMType::PUBSUB)    
    {
        method = PyObject_GetAttrString(pymodule, "subscript");
    }
    else
    {
    }
    if (!method)    {
        LOG_INFO("handler not found");
        return;
    }    
    _pvm_module_t m;
    m.argc = 0;
    m.module = pymodule;
    m.method = method;
    s_pvm_module[module] = m;
}


int
labor::PVM::execute(const string & module, const labor::JsonDoc & args, labor::PVM::PVMType type)    {
    string msg;
    int code = _pvm_service_exec(module.c_str(), args, msg);
    if (code != 0)    {
        LOG_ERROR("Call Python Module - %s - fail(%d) : %s", module.c_str(), code, msg.c_str());
    }
    return code;
}