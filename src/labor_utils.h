#ifndef __LABOR_UTILS_H__
#define __LABOR_UTILS_H__

#include <vector>
#include <string>

namespace labor
{
    std::vector<std::string> readInstalledHandler();
    std::string readConfig(const std::string & group, const std::string & name, const std::string& dval="");
}

#endif