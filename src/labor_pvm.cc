#include "labor_pvm.h"
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
void
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

void
labor::PVM::setupEnv(const vector<string> & paths)  {

}