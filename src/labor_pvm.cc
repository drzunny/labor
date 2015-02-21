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
    PyObject * module;
    PyObject * method;
};


static const char * _pvm_script_change_cwd = "import os, sys\n__pvm_cwd = os.path.abspath('$file')\nsys.path[0] = __pvm_cwd\nos.chdir(__pvm_cwd)";
static string _pvm_service_root = labor::readConfig("services.service_path", "./services");
static Hashtable<string, _pvm_module_t> s_pvm_module = Hashtable<string, _pvm_module_t>();
static PyObject * s_pvm_json = NULL;
static PyObject * s_pvm_jsonload = NULL;


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


static PyObject *
_pvm_build_args(const char * module, const char * args, const char * header, const char * ver)  {
    PyObject * vars = PyDict_New();
    PyObject * vModule = Py_BuildValue("u", module);
    PyObject * tArgs = Py_BuildValue("u", args);
    PyObject * tHeader = header != NULL ? Py_BuildValue("u", header) : NULL;
    PyObject * vVersion = ver != NULL ? Py_BuildValue("u", ver) : NULL;
    PyObject * vArgs = NULL, *vHeader = NULL;

    // Convert the `args` and `headers` to Dict object
    // find ujson - simplejson - json module.
    if (s_pvm_json == NULL) {
        s_pvm_json = PyImport_ImportModule("ujson");
        if (s_pvm_json != NULL)
            goto SETUP_ARGS;
        s_pvm_json = PyImport_ImportModule("simplejson");
        if (s_pvm_json != NULL)
            goto SETUP_ARGS;
        s_pvm_json = PyImport_ImportModule("json");
        s_pvm_jsonload = PyObject_GetAttrString(s_pvm_json, "loads");
    }

    // parse json to a dict, check args is empty or not
    if (strcmp(args, "") != 0)
        vArgs = PyObject_CallFunction(s_pvm_jsonload, "s", args);
    if (header != NULL)
        vHeader = PyObject_CallFunction(s_pvm_jsonload, "s", header);


SETUP_ARGS:
    // Set to Dict
    PyDict_SetItemString(vars, "module", vModule);
    PyDict_SetItemString(vars, "args", vArgs);
    if (header != NULL) PyDict_SetItemString(vars, "header", vHeader);
    if (ver != NULL) PyDict_SetItemString(vars, "version", vVersion);

    // Decr the ref of value
    Py_DECREF(tArgs);
    Py_DECREF(vModule);
    if (vArgs != NULL) { Py_DECREF(vArgs); }
    if (vHeader != NULL) { Py_DECREF(tHeader); Py_DECREF(vHeader); }
    if (vVersion != NULL) { Py_DECREF(vVersion); }

    return vars;
}


static int
_pvm_service_exec(const string & module, const string & args, string & msg)    {
    if (s_pvm_module.find(module) == s_pvm_module.end())
    {
        msg = "not found";
        return 404;
    }
    auto & m = s_pvm_module[module];
    // method format, next version header and ver will not be not ~
    PyObject * vars = _pvm_build_args(module.c_str(), args.c_str(), NULL, NULL);

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
    m.module = pymodule;
    m.method = method;
    s_pvm_module[module] = m;
}


int
labor::PVM::execute(const string & module, const string & args, labor::PVM::PVMType type)    {
    string msg;
    int code = _pvm_service_exec(module, args, msg);
    if (code != 0)    {
        LOG_ERROR("Call Python Module - %s - fail(%d) : %s", module.c_str(), code, msg.c_str());
    }
    return code;
}