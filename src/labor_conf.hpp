#ifndef __LABOR_CONF_H__
#define __LABOR_CONF_H__

#include "labor_opt.h"

#include <assert.h>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "labor_utils.h"

namespace labor
{
    class Conf
    {
        typedef std::string Str;
        typedef std::unordered_map<Str, Str> SHashtable;

    public:
        Conf() { error_ = false; }

        void readFile(const Str & filename) {
            std::ifstream fs(filename);
            Str ln, section;
            SHashtable conf;

            while (std::getline(fs, ln) && !error_)
            {
                error_ = this->_line_parse(std::move(ln), conf, section);
            }

            // if no error, merge the result
            if (error_) return;
            for (auto iter : conf)
            {
                val_[iter.first] = iter.second;
            }
        }


        bool hasError() const { return error_; }
        Str & get(const Str & name)  { return val_[name]; }
        void set(const Str & name, const Str & v)    { val_[name] = v; }

    private:
        Conf(const Conf & other) {}
        bool error_;
        SHashtable val_;

        void _trim_line(Str & ln)   {
            if (ln.length() == 0) return;
            size_t s = ln.find_first_not_of("\r\n\t\a\b\f\v ");
            size_t e = ln.find_last_not_of("\r\n\t\a\b\f\v ");
            if (s == Str::npos) return;
            ln = Str(ln, s, e - s + 1);
        }

        inline void _replace_macro(Str & ln)    {
            labor::string_replace(ln, "@LABOR_ROOT", labor::Options::laborRoot());
        }

        bool _line_parse(Str && ln, SHashtable & conf, Str & section)   {
            const char c_comment = '#', c_section_start = '[', c_section_end = ']', c_assign = '=';
            Str cur_section, cur_key, cur_val;
            int iter = 0;

            bool section_open = false;
            bool section_close = false;
            bool check_comment = false;
            int assign_ready = -1;

            this->_trim_line(ln);
            this->_replace_macro(ln);

            for (auto c : ln)
            {
                // if current line is a section but it has been closed
                if (section_close)
                    return true;

                switch (c)
                {
                case c_comment:
                    check_comment = true;
                    break;
                case c_section_start:
                    if (section_open || iter != 0)
                        return true;
                    section_open = true;
                    break;
                case c_section_end:
                    // if no section open
                    if (!section_open)
                        return true;
                    section_close = true;
                    cur_section = ln.substr(1, iter-1);
                    break;
                case c_assign:
                    if (section_open || assign_ready >= 0)
                        return true;
                    assign_ready = iter;
                    cur_key = ln.substr(0, iter);
                    break;
                }
                iter++;
                if (check_comment)
                    break;
            }
            // check if section not close
            if (section_open)
            {
                if (section_close)
                    section.swap(cur_section);
                else
                    return true;
            }
            if (assign_ready >= 0)
            {
                if (assign_ready + 1 >= (int)ln.length())
                    cur_val = "";
                else
                    cur_val = ln.substr(assign_ready + 1);

                this->_trim_line(cur_key);
                this->_trim_line(cur_val);
                if (section.length() > 0)   {
                    cur_key = section + Str(".") + cur_key;
                }
                val_[cur_key] = cur_val;
            }

            return false;
        }
    };
}

#endif