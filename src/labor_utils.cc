#include "labor_utils.h"
#include "labor_opt.h"
#include "labor_def.h"

#include <io.h>
#include <string.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#ifdef WIN32
#   include <Windows.h>
#else
#   include <unistd.h>
#   include <stdlib.h>
#endif

using namespace std;
using namespace boost::property_tree;


/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/

// helper macros
#define _SET_IF_NOT_NULL(p, v) if (p != NULL) *(p) = v


// Save the *.conf file's setting
static ptree * s_conf_properties;
static string  s_labor_conf;


static void
_default_conf_assignment(ptree & conf)  {
    conf.add("services.service_path", "./services");
    conf.add("labor.pubsub_addr", "127.0.0.1:1808");
}


static inline ptree &
_read_ini_config(const string & file, bool * ok = NULL)
{
    static bool __is_init = false;
    if (!__is_init)
    {
        __is_init = true;
        s_conf_properties = new ptree();
        if (!labor::path_exists(file))
        {
            _SET_IF_NOT_NULL(ok, false);
            _default_conf_assignment(*s_conf_properties);
            return *s_conf_properties;
        }
        read_ini(file, *s_conf_properties);
    }
    _SET_IF_NOT_NULL(ok, true);
    return *s_conf_properties;
}

static vector<string>
_lookup_module_dirs(const string & modulePath)
{
    return vector<string>();
}


/* ------------------------------------
* The Class Implementation
* ------------------------------------
*/
bool
labor::path_exists(const string & file)
{
#if WIN32
    return _access(file.c_str(), 0) == 0;
#else
    return access(file.c_str(), 0) == 0;
#endif
}


// labor's common
// -----------------------------------
vector<string>
labor::conf_modules() {
    auto module_path = labor::conf_read("services.service_path");
    vector<string> modules = _lookup_module_dirs(module_path);

    return modules;
}


string
labor::conf_read(const string & name, const string & dval) {
    bool ok = true;
    auto ini = _read_ini_config(labor::Options::ConfigFile().c_str(), &ok);

#ifndef LABOR_DEBUG
    // if use Labor as production,
    LABOR_ASSERT(ok == true, "labor.conf cannot found!");
#endif

    if (ini.count(name) == 0)   {
        ini.add(name, dval);
        return dval;
    }
    string v = ini.get<string>(name);
    if (v.empty()) return dval;
    return v;
}


// For String
// -----------------------------------------
vector<string>
labor::string_split(const string & s, const string & delm)    {
    vector<string> result;
    boost::split(result, s, boost::is_any_of(delm));
    return result;
}


void
labor::string_replace(string & src, const string & old, const string & now)   {
    boost::replace_all(src, old, now);
}

// For time
// ----------------------------------------
uint64_t
labor::timestamp_now()  {
    return time(0);
}


uint64_t
labor::timestamp_now_m()  {
    LABOR_ASSERT(false, "the milliseconds version is not implemented now...");
    return 0;
}


string
labor::time_now_string()   {
    char buf[25];
    time_t now = time(0);

    strftime(buf, 25, "%Y-%m-%d %X", localtime(&now));
    return __S(buf);
}


string
labor::time_utc_now_string()   {
    LABOR_ASSERT(false, "the UTC version is not implemented now...");
    return "";
}


void
labor::time_sleep(int msecs)    {
#if WIN32
    Sleep(msecs);
#else
    usleep(msecs * 1000);
#endif
}


// For path
// ---------------------------------------
string 
labor::path_getfull(const string & relpath)    {    
    char buff[256];
#if WIN32
    GetFullPathNameA(relpath.c_str(), 256, buff, NULL);
#else
    realpath(relpath.c_str(), buff);
#endif
    return string(buff);
}


void
labor::path_chdir(const string & path)  {
    string fullpath = labor::path_getfull(path);

#ifdef WIN32
    SetCurrentDirectoryA(fullpath.c_str());
#else
    chdir(fullpath.c_str());
#endif
}

// For Json
// ---------------------------------------
template<typename T>
static inline void
_set_json_object(rapidjson::Document & d, const char * k, T && v)  {
    using namespace rapidjson;
    Value key(StringRef(k));
    Value val(std::move(v));
    d.AddMember(key, val, d.GetAllocator());
}


static inline const rapidjson::Value &
_check_json_object(const rapidjson::Document & d, const char * key = "") {
    if (strcmp(key, "") != 0)
        return d[key];
    return d;
}

static void
_get_json_object(rapidjson::Document & d, rapidjson::Value & v)  {
    if (v.IsInt())
        d.SetInt(v.GetInt());
    if (v.IsInt64())
        d.SetInt64(v.GetInt64());
    if (v.IsString())
        d.SetString(rapidjson::StringRef(v.GetString()));
    if (v.IsDouble())
        d.SetDouble(v.GetDouble());
    if (v.IsBool())
        d.SetBool(v.GetBool());
    if (v.IsArray() || v.IsObject())    {
        if (v.IsArray())
        {
            d.SetArray();
            for (auto iter = v.Begin(); iter != v.End(); iter++)
                d.PushBack(*iter, d.GetAllocator());
        }
        else
        {
            d.SetObject();
            for (auto iter = v.MemberBegin(); iter != v.MemberEnd(); iter++)
            {
                d.AddMember(iter->name, iter->value, d.GetAllocator());
            }
        }
    }
}

class labor::_jsondoc_impl
{
public:
    _jsondoc_impl() {}
    _jsondoc_impl(rapidjson::Document & doc) : d_(std::move(doc)) {}
    ~_jsondoc_impl() {}

    rapidjson::Document & raw() { return d_; }
    bool isNull() { return d_.IsNull(); }
    bool has(const string & name) { return d_.HasMember(name.c_str()); }

    void set(const string & name, const string & s) { _set_json_object(d_, name.c_str(), rapidjson::StringRef(s.c_str())); }
    void set(const string & name, int i) { _set_json_object(d_, name.c_str(), i); }
    void set(const string & name, int64_t i)    { _set_json_object(d_, name.c_str(), i); }
    void set(const string & name, double d) { _set_json_object(d_, name.c_str(), d); }
    void set(const string & name, bool b) { _set_json_object(d_, name.c_str(), b); }
    void set(const string & name, rapidjson::Document & d) { _set_json_object(d_, name.c_str(), d); }

    void push(const string & name, const string & v)    {
        if (name.empty())
            d_.PushBack(rapidjson::StringRef(v.c_str()), d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(rapidjson::StringRef(v.c_str()), d_.GetAllocator());
    }
    void push(const string & name, int v)    {
        if (name.empty())
            d_.PushBack(v, d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(v, d_.GetAllocator());
    }
    void push(const string & name, int64_t v)    {
        if (name.empty())
            d_.PushBack(v, d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(v, d_.GetAllocator());
    }
    void push(const string & name, double v)    {
        if (name.empty())
            d_.PushBack(v, d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(v, d_.GetAllocator());
    }
    void push(const string & name, bool v)    {
        if (name.empty())
            d_.PushBack(v, d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(v, d_.GetAllocator());
    }
    void push(const string & name, rapidjson::Document & v)    {
        if (name.empty())
            d_.PushBack(v, d_.GetAllocator());
        else
            d_[rapidjson::Value(rapidjson::StringRef(name.c_str()))].PushBack(v, d_.GetAllocator());
    }

    rapidjson::Value & get(const string & name) {
        rapidjson::Value key(rapidjson::StringRef(name.c_str()));
        return d_[key];
    }

    rapidjson::Value & getIndex(int i)    {
        return d_[i];
    }

    size_t count() const {
        return d_.MemberCount();
    }

    int toInt() const { return d_.GetInt(); }
    int64_t toInt64() const { return d_.GetInt64(); }
    string toString() const { return d_.GetString(); }
    double toDouble() const { return d_.GetDouble(); }
    bool toBool() const { return d_.GetBool(); }

    bool isInt(const std::string & name = "") const { auto & v = _check_json_object(d_, name.c_str()); return v.IsInt(); }
    bool isInt64(const std::string & name = "") const   { auto & v = _check_json_object(d_, name.c_str()); return v.IsInt64(); }
    bool isString(const std::string & name = "") const  { auto & v = _check_json_object(d_, name.c_str()); return v.IsString(); }
    bool isDouble(const std::string & name = "") const  { auto & v = _check_json_object(d_, name.c_str()); return v.IsDouble(); }
    bool isBool(const std::string & name = "") const    { auto & v = _check_json_object(d_, name.c_str()); return v.IsBool(); }

private:
    rapidjson::Document d_;
    _jsondoc_impl(const _jsondoc_impl & o) {}
};

// Set Values
void labor::JsonDoc::set(const string & name, const string & v) { doc_->set(name, v); }
void labor::JsonDoc::set(const string & name, int v) { doc_->set(name, v); }
void labor::JsonDoc::set(const string & name, int64_t v) { doc_->set(name, v); }
void labor::JsonDoc::set(const string & name, double v) { doc_->set(name, v); }
void labor::JsonDoc::set(const string & name, bool v) { doc_->set(name, v); }
void labor::JsonDoc::set(const string & name, labor::JsonDoc & v)  { doc_->set(name, v.doc_->raw()); }

void labor::JsonDoc::push(const string & name, const string & v) { doc_->push(name, v); }
void labor::JsonDoc::push(const string & name, int v) { doc_->push(name, v); }
void labor::JsonDoc::push(const string & name, int64_t v) { doc_->push(name, v); }
void labor::JsonDoc::push(const string & name, double v) { doc_->push(name, v); }
void labor::JsonDoc::push(const string & name, bool v) { doc_->push(name, v); }
void labor::JsonDoc::push(const string & name, labor::JsonDoc & v) { doc_->push(name, v.doc_->raw()); }

// Get values
labor::JsonDoc::JsonDoc() : doc_(new labor::_jsondoc_impl()) { }
labor::JsonDoc::~JsonDoc() {}
bool labor::JsonDoc::isNull() const  { return doc_->isNull(); }
bool labor::JsonDoc::has(const string & name) const  { return doc_->has(name); }
int labor::JsonDoc::toInt() const { return doc_->toInt(); }
int64_t labor::JsonDoc::toInt64() const { return doc_->toInt64(); }
string labor::JsonDoc::toString() const { return doc_->toString(); }
double labor::JsonDoc::toDouble() const { return doc_->toDouble(); }
bool labor::JsonDoc::toBool() const { return doc_->toBool(); }

labor::JsonDoc
labor::JsonDoc::get(const string & name) const    {
    auto r = labor::JsonDoc();
    rapidjson::Value & v = doc_->get(name);    
    _get_json_object(r.doc_->raw(), v);

    return r;
}


labor::JsonDoc
labor::JsonDoc::getIndex(int i) const   {
    auto r = labor::JsonDoc();
    rapidjson::Value & v = doc_->getIndex(i);
    _get_json_object(r.doc_->raw(), v);

    return r;
}


size_t
labor::JsonDoc::count() const {
    return doc_->count();
}

// Check Values
bool labor::JsonDoc::isInt(const string & name)const { return doc_->isInt(name); }
bool labor::JsonDoc::isInt64(const string & name) const { return doc_->isInt64(name); }
bool labor::JsonDoc::isString(const string & name)const { return doc_->isString(name); }
bool labor::JsonDoc::isDouble(const string & name) const { return doc_->isDouble(name); }
bool labor::JsonDoc::isBool(const string & name) const { return doc_->isBool(name); }

string
labor::JsonDoc::encode(const labor::JsonDoc & doc)   {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    rapidjson::Document & d = doc.doc_->raw();
    d.Accept(writer);
    return sb.GetString();
}


labor::JsonDoc
labor::JsonDoc::decode(const string & json)  {
    JsonDoc d;
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    d.doc_ = unique_ptr<labor::_jsondoc_impl>(new labor::_jsondoc_impl(doc));
    return d;
}