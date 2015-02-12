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
    void string_replace(std::string & src, const std::string & old, const std::string & now);



    // for time
    // --------------------------------------------
    uint64_t timestamp_now();
    uint64_t timestamp_now_m();
    std::string time_now_string();
    std::string time_utc_now_string();


    // for Json
    // --------------------------------------------
    class _jsondoc_impl;
    class JsonDoc
    {
    public:
        // TODO: ADD RValue support
        JsonDoc();
        JsonDoc(JsonDoc && d) { doc_ = d.doc_; };
        ~JsonDoc();

        static std::string encode(const JsonDoc & doc);
        static JsonDoc decode(const std::string & json);
        
        bool has(const std::string & name) const;
        bool isNull() const;

        template<typename T>
        void set(const std::string & name, T & s);
        template<typename T> 
        void push(const std::string & name, T & val);


        JsonDoc get(const std::string & name);
        JsonDoc getIndex(int i);

        // Get values
        int toInt() const;
        int64_t toInt64() const;
        std::string toString() const;
        double toDouble() const;
        bool toBool() const;

        // Check Values
        bool isInt(const std::string & name = "") const;
        bool isInt64(const std::string & name = "") const;
        bool isString(const std::string & name = "") const;
        bool isDouble(const std::string & name = "") const;
        bool isBool(const std::string & name = "") const;

    private:
        std::shared_ptr<_jsondoc_impl> doc_;
        JsonDoc::JsonDoc(const JsonDoc & o) {};
    };
}

#endif