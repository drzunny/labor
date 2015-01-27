#ifndef __LABOR_UTILS_H__
#define __LABOR_UTILS_H__

#include <vector>
#include <string>
#include <memory>
#include <time.h>
#include <stdint.h>

#define FREE_IF_NOT_NULL(p) \
    if (p != NULL) {free(p);} \
    p = NULL

namespace labor
{
    // About labor's common function
    // --------------------------------------------
    void ready(int argc, char * argv[]);
    bool fileExists(const std::string & file);
    std::vector<std::string> readInstallModules();
    std::string readConfig(const std::string & name, const std::string& dval="");



    // for string
    // --------------------------------------------
    std::vector<std::string> string_split(const std::string & s, const std::string & delm);



    // for time
    // --------------------------------------------
    uint64_t timestamp_now();
    uint64_t timestamp_now_m();


    // for Json
    // --------------------------------------------
    class _jsondoc_impl;
    class JsonDoc
    {
    public:
        JsonDoc();
        ~JsonDoc();

        static std::string encode(const JsonDoc & doc);
        static JsonDoc decode(const std::string & json);
        
        bool has(const std::string & name) const;
        bool isNull() const;

        JsonDoc get(const std::string & name);
        JsonDoc getIndex(int i);

        int toInt() const;
        int64_t toInt64() const;
        std::string toString() const;
        double toDouble() const;
        bool toBool() const;

    private:
        std::shared_ptr<_jsondoc_impl> doc_;
        labor::JsonDoc::JsonDoc(const labor::JsonDoc & o) {};
    };
}

#endif