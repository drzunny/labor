#include "labor_pvm.h"
#include <Python.h>

/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/


/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
labor::PvmPtr
labor::PVM::init()  {
    // Init the Python VM. Fuck GIL.
    if (!Py_IsInitialized())    {
        Py_Initialize();
        // TODO: setup the environments
    }
    // todo, load target service
}

void
labor::PVM::dispose()   {
    Py_Finalize();
}