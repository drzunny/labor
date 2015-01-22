#ifndef __LABOR_UTILS_H__
#define __LABOR_UTILS_H__

#include <vector>
#include <string>

#define FREE_IF_NOT_NULL(p) \
    if (p != NULL) {free(p);} \
    p = NULL

namespace labor
{
    // About labor's common function
    void ready(int argc, char * argv[]);
    bool fileExists(const std::string & file);
    std::vector<std::string> readInstallModules();
    std::string readConfig(const std::string & name, const std::string& dval="");

    // for string
    std::vector<std::string> string_split(const std::string & s, char delm);
}

#endif