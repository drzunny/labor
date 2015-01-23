#include "labor.h"
#include <stdio.h>

//-----------------------------------
// Ready
//-----------------------------------
static bool
labor_prepare(int argc, char * argv[])    {
    labor::PVM::init();

    return false;
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
    labor::Event ev;
    ev.run();
    return 0;
}