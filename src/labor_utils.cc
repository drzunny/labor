#include "labor_utils.h"

#include <io.h>
#include <algorithm>
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
labor::string_split(const string & s, char delm)    {
    // FIXIT: this is not a efficient way to split
    istringstream iss(s);
    istringstream dm(delm);
    vector<string> out;

    std::copy(istream_iterator<string>(iss), istream_iterator<string>(dm), std::back_inserter(out));

    return out;
}

// For Json
// ---------------------------------------
class labor::_jsondoc_impl
{
public:
    _jsondoc_impl() {}
    _jsondoc_impl(rapidjson::Document & doc) : d_(std::move(doc)) {}
    ~_jsondoc_impl() {}

    rapidjson::Document & raw() { return d_; }
    bool isNull() { return d_.IsNull(); }
    bool has(const string & name) { return d_.HasMember(name.c_str()); }

    int toInt() const { return d_.GetInt(); }
    int64_t toInt64() const { return d_.GetInt64(); }
    string toString() const { return d_.GetString(); }
    double toDouble() const { return d_.GetDouble(); }
    bool toBool() const { return d_.GetBool(); }

private:
    rapidjson::Document d_;
    _jsondoc_impl(const _jsondoc_impl & o) {}
};


labor::JsonDoc::JsonDoc() : doc_(new labor::_jsondoc_impl()) { }
labor::JsonDoc::~JsonDoc() {}
bool labor::JsonDoc::isNull() const  { return doc_->isNull(); }
bool labor::JsonDoc::has(const string & name) const  { return doc_->has(name); }
int labor::JsonDoc::toInt() const { return doc_->toInt(); }
int64_t labor::JsonDoc::toInt64() const { return doc_->toInt64(); }
string labor::JsonDoc::toString() const { return doc_->toString(); }
double labor::JsonDoc::toDouble() const { return doc_->toDouble(); }
bool labor::JsonDoc::toBool() const { return doc_->toBool(); }

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