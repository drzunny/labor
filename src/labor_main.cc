#include "labor.h"
#include <stdio.h>

//-----------------------------------
// Ready
//-----------------------------------
#define LABOR_OPERATION_START(op) \
    printf("startup operation<%s>....\t\t", #op);\
    if ((op)) printf("[ OK ]\n");\
        else  { printf("[FAIL]\n"); ok = false;}

static const char * s_running_mode[] = { "DEBUG", "NORMAL", "OPTIMIZED" };

/*
    Display running mode
*/
static const char *
_labor_running_mode()   {
    auto mode = labor::Options::runningMode();
    return s_running_mode[(int)mode];
}


/*
    Prepare the service
*/
static bool
_labor_prepare(int argc, char * argv[])    {
    bool ok = true;
    if (labor::Options::enablePython()){
        LABOR_OPERATION_START(labor::PVM::init())
    }
    if (labor::Options::enableLua())    {
        LABOR_OPERATION_START(labor::LVM::init())
    }
    LABOR_OPERATION_START(labor::Logger::init())

    printf("\nlabor.conf's path:      \"%s\"\nrunning mode:           \"%s\"\n\n",
        labor::Options::ConfigFile().c_str(), _labor_running_mode());
    return true;
}


/*
    Release the modules
*/
static void
_labor_release() {
    if (labor::Options::enablePython()){
        labor::PVM::dispose();
    }
    if (labor::Options::enableLua())    {
        labor::LVM::dispose();
    }
    labor::Logger::dispose();
}


/*
    Startup and say hello
*/
static void
_labor_sayhello()    {
    printf("\n\
#------------------------------------------\n\
# labor start!\n\
#------------------------------------------\n\n");
}

//-----------------------------------
// Labor startup!
//-----------------------------------
int main(int argc, char * argv[])
{
    labor::Options::parse(argc, argv);

    // is the running options use '-v/--version' or  '-h/--help'
    if (labor::Options::checkAndShowHelp())
        return 0;
    if (labor::Options::checkAndShowVersion())
        return 0;

    // Prepare
    _labor_sayhello();
    bool ready = _labor_prepare(argc, argv);
    if (!ready)
    {
        printf("labor prepare is fail....\n");
        return -1;
    }    

    for (int i = 0; i < 10000; i++) { LOG_DEBUG("Hello world"); }

    // start the event loop
    labor::Event ev;
    ev.run();
    _labor_release();
    return 0;
}