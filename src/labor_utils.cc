#include "labor_utils.h"

#include <io.h>
#include <string.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

using namespace std;
using namespace boost::property_tree;


/* ------------------------------------
* The Helper Functions
* ------------------------------------
*/

// helper macros
#define _SET_IF_NOT_NULL(p, v) if (p != NULL) *##p = v


// Save the *.conf file's setting
static ptree s_conf_properties;
static string s_labor_conf;


static inline ptree &
_read_ini_config(const string & file, bool * ok = NULL)
{
    if (s_conf_properties.empty())
    {
        if (!labor::fileExists(file))
        {
            _SET_IF_NOT_NULL(ok, false);
            return s_conf_properties;
        }
        read_ini(file, s_conf_properties);
    }
    _SET_IF_NOT_NULL(ok, true);
    return s_conf_properties;
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
void
labor::ready(int argc, char * argv[])
{
#ifndef LABOR_CONF
    s_labor_conf = "";
#else
    s_labor_conf = LABOR_CONF;
#endif
}


bool
labor::fileExists(const string & file)
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
labor::readInstallModules() {
    auto module_path = labor::readConfig("services.service_path");
    vector<string> modules = _lookup_module_dirs(module_path);

    return modules;
}


string
labor::readConfig(const string & name, const string & dval) {
    bool ok = true;
    auto ini = _read_ini_config("", &ok);
    assert(ok == true);

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


// For time
// ----------------------------------------
uint64_t
labor::timestamp_now()  {
    return time(0);
}


uint64_t
labor::timestamp_now_m()  {
    BOOST_ASSERT_MSG(false, "the milliseconds version is not implemented now...");
    return 0;
}


// For Json
// ---------------------------------------
template<typename T>
static inline void
_set_json_object(rapidjson::Document & d, const char * k, T && v)  {
    using namespace rapidjson;
    Value key(StringRef(k));
    Value val(v);
    d.AddMember(key, val, d.GetAllocator());
}


static inline const rapidjson::Value &
_check_json_object(const rapidjson::Document & d, const char * key = "") {
    if (strcmp(key, "") == 0)
        return d[key];
    return d;
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

    void set(const string & name, string & s) { _set_json_object(d_, name.c_str(), rapidjson::StringRef(s.c_str())); }
    void set(const string & name, int i) { _set_json_object(d_, name.c_str(), i); }
    void set(const string & name, int64_t i)    { _set_json_object(d_, name.c_str(), i); }
    void set(const string & name, double d) { _set_json_object(d_, name.c_str(), d); }
    void set(const string & name, bool b) { _set_json_object(d_, name.c_str(), b); }
    void set(const string & name, rapidjson::Document & d) { _set_json_object(d_, name.c_str(), d); }

    void push(const string & name, const string & v)    {}
    void push(const string & name, int v)    {}
    void push(const string & name, int64_t v)    {}
    void push(const string & name, double v)    {}
    void push(const string & name, bool v)    {}
    void push(const string & name, rapidjson::Document & v)    {}

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
template<string> void labor::JsonDoc::set(const string & name, string & v) { doc_->set(name, v); }
template<int> void labor::JsonDoc::set(const string & name, int & v) { doc_->set(name, v); }
template<int64_t> void labor::JsonDoc::set(const string & name, int64_t & v) { doc_->set(name, v); }
template<double> void labor::JsonDoc::set(const string & name, double & v) { doc_->set(name, v); }
template<bool> void labor::JsonDoc::set(const string & name, bool & v) { doc_->set(name, v); }
template<labor::JsonDoc> void labor::JsonDoc::set(const string & name, labor::JsonDoc & v)  { doc_->set(name, v); }

template<string> void labor::JsonDoc::push(const string & name, string & v) { doc_->push(name, v); }
template<int> void labor::JsonDoc::push(const string & name, int & v) { doc_->set(push, v); }
template<int64_t> void labor::JsonDoc::push(const string & name, int64_t & v) { doc_->push(name, v); }
template<double> void labor::JsonDoc::push(const string & name, double & v) { doc_->push(name, v); }
template<bool> void labor::JsonDoc::push(const string & name, bool & v) { doc_->push(name, v); }
template<labor::JsonDoc> void labor::JsonDoc::push(const string & name, labor::JsonDoc & v) { doc_->push(name, v); }

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
labor::JsonDoc::get(const string & name)    {
    return labor::JsonDoc();
}


labor::JsonDoc
labor::JsonDoc::getIndex(int i)   {
    return labor::JsonDoc();
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