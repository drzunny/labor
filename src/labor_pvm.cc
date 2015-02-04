#include "labor_pvm.h"
#include "labor_log.h"

#include <Python.h>

using namespace std;
/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


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
        // TODO: setup the environments
    }
    // todo, load target service

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
labor::PVM::setupEnv(const vector<string> & paths)  {

}