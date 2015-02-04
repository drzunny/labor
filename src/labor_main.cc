#include "labor.h"
#include <stdio.h>

//-----------------------------------
// Ready
//-----------------------------------
#define LABOR_OPERATION_START(op) \
    printf("startup operation<%s>........\t\t", #op);\
    if ((op)) printf("[ OK ]\n");\
    else  { printf("[FAIL]\n"); ok = false;}

static bool
labor_prepare(int argc, char * argv[])    {
    bool ok = true;
    LABOR_OPERATION_START(labor::PVM::init())
    LABOR_OPERATION_START(labor::Options::parse(argc, argv))
    return true;
}

static void
labor_release() {

}

//-----------------------------------
// Labor startup!
//-----------------------------------
int main(int argc, char * argv[])
{
    // Init Python
    // Start Event loop
    // Prepare the callbacks
    bool ready = labor_prepare(argc, argv);
    if (!ready)
    {
        printf("labor prepare is fail....\n");
        return -1;
    }

    // is the running options use '-v/--version' or  '-h/--help'
    if (labor::Options::checkAndShowHelp())
        return 0;
    if (labor::Options::checkAndShowVersion())
        return 0;

    // start the event loop
    labor::Event ev;
    ev.run();
    return 0;
}