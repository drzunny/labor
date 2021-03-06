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

    // The logger::init must be the first. `cwd` will be changed after this;
    LABOR_OPERATION_START(labor::Logger::init());
    LABOR_OPERATION_START(labor::Service::setEnv());
    if (labor::Options::enablePython()) {
        LABOR_OPERATION_START(labor::PVM::init());
    }
    if (labor::Options::enableLua())    {
        LABOR_OPERATION_START(labor::LVM::init());
    }

        printf("\nlabor.conf's path:      \"%s\"\nrunning mode:           \"%s\"\nlog directory:          \"%s\"\n\n",
        labor::Options::configFile().c_str(), _labor_running_mode(), labor::Logger::filePath().c_str());
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
    printf("\n\n");

    // Different output for different platform
#ifndef WIN32
    printf(" ┌─┐      ┌──────┐ ┌─────+  ┌──────┐ ┌────+\n");
    printf(" │ │      │ ┌──┐ │ │ ┌──┐ \\ │ ┌──┐ │ │ ┌─┐ \\\n");
    printf(" │ │      │ └──┘ │ │ └──┘ / │ │  │ │ │ └─┘ /\n");
    printf(" │ │      │ ┌──┐ │ │ ┌──┐ \\ │ │  │ │ │ ┌─┐ +\n");
    printf(" │ └────┐ │ │  │ │ │ └──┘ / │ └──┘ │ │ │ │ │\n");
    printf(" └──────┘ └─┘  └─┘ └─────+  └──────┘ └─┘ └─┘\n\n\n");
#else
    printf(" .-.      .------. .-----.  .------. .----.\n");
    printf(" | |      | .--. | | .--. \\ | .--. | | .-. \\\n");
    printf(" | |      | '--' | | '--' / | |  | | | '-' /\n");
    printf(" | |      | .--. | | .--. \\ | |  | | | .-. '\n");
    printf(" | '----. | |  | | | '--' / | '--' | | | | |\n");
    printf(" '------' '-'  '-' '-----'  '------' '-' '-'\n\n\n");
#endif
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

    //for (int i = 0; i < 10000; i++) { LOG_INFO("Hello"); }

    // start the event loop
    labor::Event ev;
    ev.run();
    _labor_release();
    return 0;
}
