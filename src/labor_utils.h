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
    std::vector<std::string> conf_modules();
    std::string conf_read(const std::string & name, const std::string& dval="");



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
    void time_sleep(int msecs);


    
    // for path
    // --------------------------------------------
    bool path_exists(const std::string & file);
    std::string path_getfull(const std::string & relpath);
    void path_chdir(const std::string & path);



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

        void set(const std::string & name, const std::string & s);
        void set(const std::string & name, int s);
        void set(const std::string & name, int64_t s);
        void set(const std::string & name, double s);
        void set(const std::string & name, bool s);
        void set(const std::string & name, JsonDoc & s);

        void push(const std::string & name, const std::string & val);
        void push(const std::string & name, int val);
        void push(const std::string & name, int64_t val);
        void push(const std::string & name, double val);
        void push(const std::string & name, bool val);
        void push(const std::string & name, JsonDoc & val);


        JsonDoc get(const std::string & name) const;
        JsonDoc getIndex(int i) const;
        size_t count() const;

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
        JsonDoc(const JsonDoc & o) {};
    };
}

#endif