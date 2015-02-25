#include "labor_pvm.h"
#include "labor_log.h"
#include "labor_utils.h"
#include "labor_def.h"

#include <unordered_map>

#include <Python.h>
#include <frameobject.h>

using namespace std;
/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/
#ifdef LABOR_DEBUG
#  define __DECRREF(x)
#  define __XDECRREF(x)
#else
#  define __DECRREF(x)  if (x!=NULL) {Py_DECREF(x);}
#  define __XDECRREF(x)  if (x!=NULL) {Py_XDECREF(x);}
#endif

struct _pvm_module_t
{
    PyObject * module;
    PyObject * method;
};


static Hashtable<string, _pvm_module_t> s_pvm_module = Hashtable<string, _pvm_module_t>();
static PyObject * s_pvm_json = NULL;
static PyObject * s_pvm_jsonload = NULL;


/* ------------------------------------
* The PVM Helpers
* ------------------------------------
*/

static PyObject *
_pvm_build_args(const char * module, const char * args, const char * header, const char * ver)  {
    PyObject * tp = PyTuple_New(1);
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
    if (vArgs != NULL) PyDict_SetItemString(vars, "args", vArgs);
    if (header != NULL) PyDict_SetItemString(vars, "header", vHeader);
    if (ver != NULL) PyDict_SetItemString(vars, "version", vVersion);

    PyTuple_SetItem(tp, 0, vars);

    // Decr the ref of value
    __DECRREF(tArgs);
    __DECRREF(vModule);
    __DECRREF(vArgs);
    __DECRREF(tHeader);
    __DECRREF(vHeader);
    __DECRREF(vVersion);
    __DECRREF(vars);

    return tp;
}


static void
_pvm_error_readline(const char * filename, int line, string * msg)  {
    FILE * f = fopen(filename, "r");
    if (f == NULL)  {
        (*msg).append("\n");
        return;
    }
    char linebuf[1024];
    int ln = 1;
    while (fgets(linebuf, 1024, f)) {
        if (ln != line) {
            ln++;
            continue;
        }
        char * ch = linebuf;
        (*msg).append(linebuf);
        break;
    }
    //(*msg).append("\n");
    fclose(f);
}


static int
_pvm_error_trackback(PyObject * trace, long * limit, string * msg)    {
    long depth = 0;
    int err = 0;
    PyTracebackObject * tb, *tb1;
    PyObject * limitObj = PySys_GetObject("tracebacklimit");
    if (limitObj && PyInt_Check(limitObj))
    {
        if ((*limit = PyInt_AsLong(limitObj) <= 0))
            return -2;
    }
    (*msg).append("\n");
    tb = (PyTracebackObject *)trace;
    tb1 = tb;
    while (tb1 != NULL) { depth++; tb1 = tb1->tb_next; }
    while (tb != NULL && err == 0)
    {
        if (depth <= *limit) {
            char linebuf[2000];
            sprintf(linebuf, "File \"%.500s\", Line %d, in %.500s\n",
                PyString_AsString(tb->tb_frame->f_code->co_filename),
                tb->tb_lineno,
                PyString_AsString(tb->tb_frame->f_code->co_name)
                );
            (*msg).append(linebuf);
        }
        _pvm_error_readline(PyString_AsString(tb->tb_frame->f_code->co_filename), tb->tb_lineno, msg);
        depth--;
        tb = tb->tb_next;
        if (err == 0)
            err = PyErr_CheckSignals();
    }
    // Get Current Code
    return err;
}


static void
_pvm_error_string(string & msg) {
    PyObject *pType, *pValue, *pTrace;
    int err = 0;
    long limit = 1000;

    PyErr_Fetch(&pType, &pValue, &pTrace);
    if (pType == NULL || pType == Py_None)
        return;
    PyErr_NormalizeException(&pType, &pValue, &pTrace);

    // Print Trace.
    // These operation in windows may be fail if you haven't python27_d.lib
    // it success only in Release mode. In Debug mode, pTrace is PyLong
    if (!PyTraceBack_Check(pTrace))
    {
        msg.append("Python -- Bad Internal Call");
        return;
    }
    else
    {
        err = _pvm_error_trackback(pTrace, &limit, &msg);
        if (PyExceptionClass_Check(pType))
        {
            // exception.xxxx
            char * classname = PyExceptionClass_Name(pType);
            // .xxx
            char * dot = strrchr(classname, '.');
            // dot+1 = xxx
            msg.append(dot+1);
        }
        printf("err = %d\n", err);
        if (err == 0 && pValue != Py_None)
        {
            PyObject * s = PyObject_Str(pValue);
            if (PyString_Check(s) && PyString_GET_SIZE(s) != 0)  {
                msg.append(": ");
                msg.append(PyString_AsString(s));
            }
            __DECRREF(s);
            __DECRREF(pValue);
        }
        __DECRREF(pType);
        __DECRREF(pTrace);
        PyErr_Clear();
    }
}

static int
_pvm_service_exec(const string & module, labor::PVM::PVMType type, const string & args, string & msg)    {
    string moduleKey(module);
    moduleKey.append(type == labor::PVM::PUBSUB ? "_0" : "_1");
    if (s_pvm_module.find(moduleKey) == s_pvm_module.end())
    {
        msg = "not found";
        return 404;
    }
    auto & m = s_pvm_module[moduleKey];
    // method format, next version header and ver will not be not ~
    PyObject * vars = _pvm_build_args(module.c_str(), args.c_str(), NULL, NULL);
    PyObject * ret = PyObject_Call(m.method, vars, NULL);

    __DECRREF(vars);
    if (ret != NULL)
    {
        __DECRREF(ret);
        return 0;
    }
    else if (PyErr_Occurred())
    {
        // why i cannot get these?
#ifdef LABOR_DEBUG
        PyErr_Print();
#else
        string errMsg;
        _pvm_error_string(errMsg);
        LOG_INFO("Python Service Error!");        
        LOG_ERROR("%s", errMsg);        
#endif

        return 500;
    }
    else
        return -1; // unknown

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
    string moduleKey(module);
    moduleKey.append(type == labor::PVM::PUBSUB ? "_0" : "_1");
    if (s_pvm_module.find(moduleKey) != s_pvm_module.end())
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
    s_pvm_module[moduleKey] = m;
}


int
labor::PVM::execute(const string & module, const string & args, labor::PVM::PVMType type)    {
    string msg;
    int code = _pvm_service_exec(module, type, args, msg);
    if (code != 0)    {
        LOG_ERROR("Call Python Module - %s - fail(%d) : %s", module.c_str(), code, msg.c_str());
    }
    return code;
}