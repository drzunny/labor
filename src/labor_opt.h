#ifndef __LABOR_OPTIONS_H__
#define __LABOR_OPTIONS_H__

#include "labor_def.h"
#include <string>

namespace labor
{
    class Options
    {
    public:
        enum LaborOptions
        {
            OP_Conf = 0,
            OP_EnablePython = 1,
            OP_EnableLua = 2,
            OP_Mode = 3,

            // Stop for running
            OP_Help = -1,
            OP_Version = -2
        };

        enum RunningMode
        {
            Mode_Debug = 0,
            Mode_Normal = 1,
            Mode_Optimized = 2
        };

        static bool parse(int argc, char *argv[]);

        static const std::string & configFile();
        static const std::string & laborRoot();
        static bool enablePython();
        static bool enableLua();
        static RunningMode runningMode();

        static bool checkAndShowHelp();
        static bool checkAndShowVersion();
    };
}

#endif
