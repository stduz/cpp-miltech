#pragma once
// Мінімальна реалізація nlohmann/json API для цього проекту.
// Можна замінити на оригінальний json.hpp з https://github.com/nlohmann/json/releases

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cctype>
#include <stdexcept>

namespace nlohmann {

class json {
public:
    enum class value_t { null, boolean, number, string, array, object };

    value_t           type_;
    bool              bool_;
    double            num_;
    std::string       str_;
    std::vector<json> arr_;
    std::map<std::string, json> obj_;

    json() : type_(value_t::null), bool_(false), num_(0.0) {}
    explicit json(value_t t) : type_(t), bool_(false), num_(0.0) {}

    json(bool v)               : type_(value_t::boolean), bool_(v),    num_(0.0)        {}
    json(int v)                : type_(value_t::number),  bool_(false), num_((double)v)  {}
    json(long long v)          : type_(value_t::number),  bool_(false), num_((double)v)  {}
    json(float v)              : type_(value_t::number),  bool_(false), num_((double)v)  {}
    json(double v)             : type_(value_t::number),  bool_(false), num_(v)          {}
    json(const char* v)        : type_(value_t::string),  bool_(false), num_(0.0), str_(v) {}
    json(const std::string& v) : type_(value_t::string),  bool_(false), num_(0.0), str_(v) {}

    // {{"key", val}, ...} -> object; {val, val, ...} -> array
    json(std::initializer_list<json> il) : type_(value_t::null), bool_(false), num_(0.0) {
        bool as_obj = (il.size() > 0);
        for (const auto& j : il)
            if (j.type_ != value_t::array || j.arr_.size() != 2 || j.arr_[0].type_ != value_t::string)
                { as_obj = false; break; }
        if (as_obj) {
            type_ = value_t::object;
            for (const auto& j : il) obj_[j.arr_[0].str_] = j.arr_[1];
        } else {
            type_ = value_t::array;
            arr_.assign(il.begin(), il.end());
        }
    }

    json& operator[](const char* k)        { if (type_==value_t::null) type_=value_t::object; return obj_[k]; }
    json& operator[](const std::string& k) { if (type_==value_t::null) type_=value_t::object; return obj_[k]; }
    json& operator[](int i)                { return arr_[(size_t)i]; }
    json& operator[](size_t i)             { return arr_[i]; }

    const json& operator[](const char* k)        const { return obj_.at(k); }
    const json& operator[](const std::string& k) const { return obj_.at(k); }
    const json& operator[](int i)                const { return arr_[(size_t)i]; }
    const json& operator[](size_t i)             const { return arr_[i]; }

    size_t size() const {
        if (type_==value_t::array)  return arr_.size();
        if (type_==value_t::object) return obj_.size();
        return 0;
    }

    void push_back(const json& v) { if (type_==value_t::null) type_=value_t::array; arr_.push_back(v); }

    static json array()  { return json(value_t::array);  }
    static json object() { return json(value_t::object); }

    json& operator=(bool v)               { type_=value_t::boolean; bool_=v;        return *this; }
    json& operator=(int v)                { type_=value_t::number;  num_=(double)v; return *this; }
    json& operator=(long long v)          { type_=value_t::number;  num_=(double)v; return *this; }
    json& operator=(float v)              { type_=value_t::number;  num_=(double)v; return *this; }
    json& operator=(double v)             { type_=value_t::number;  num_=v;         return *this; }
    json& operator=(const char* v)        { type_=value_t::string;  str_=v;         return *this; }
    json& operator=(const std::string& v) { type_=value_t::string;  str_=v;         return *this; }

    template<typename T> T get() const;

    template<typename T>
    T value(const std::string& k, const T& def) const {
        if (type_ != value_t::object) return def;
        auto it = obj_.find(k);
        if (it == obj_.end()) return def;
        return it->second.get<T>();
    }
    template<typename T>
    T value(const char* k, const T& def) const { return value(std::string(k), def); }

    bool contains(const std::string& k) const {
        return type_ == value_t::object && obj_.count(k);
    }

    static json parse(const std::string& s);
    std::string dump(int indent = -1, int depth = 0) const;

    friend std::istream& operator>>(std::istream& is, json& j) {
        std::string s((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        j = json::parse(s); return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const json& j) {
        return os << j.dump();
    }
};

template<> inline float       json::get<float>()       const { return (float)num_; }
template<> inline double      json::get<double>()      const { return num_; }
template<> inline int         json::get<int>()         const { return (int)num_; }
template<> inline bool        json::get<bool>()        const { return bool_; }
template<> inline std::string json::get<std::string>() const { return str_; }

// --- парсер ---
namespace detail {

inline void skip(const std::string& s, size_t& p) {
    while (p < s.size() && (s[p]==' '||s[p]=='\t'||s[p]=='\n'||s[p]=='\r')) p++;
}

inline std::string read_str(const std::string& s, size_t& p) {
    p++; std::string r;
    while (p < s.size() && s[p] != '"') {
        if (s[p] == '\\' && p+1 < s.size()) {
            p++;
            if      (s[p]=='"')  r+='"';
            else if (s[p]=='\\') r+='\\';
            else if (s[p]=='n')  r+='\n';
            else if (s[p]=='t')  r+='\t';
            else                 r+=s[p];
        } else r += s[p];
        p++;
    }
    p++; return r;
}

json read_val(const std::string& s, size_t& p);

inline json read_obj(const std::string& s, size_t& p) {
    json j(json::value_t::object); p++; skip(s,p);
    if (p < s.size() && s[p] == '}') { p++; return j; }
    for (;;) {
        skip(s,p);
        std::string key = read_str(s,p);
        skip(s,p); p++; // ':'
        j[key] = read_val(s,p);
        skip(s,p);
        if (p < s.size() && s[p] == '}') { p++; break; }
        if (p < s.size() && s[p] == ',') p++;
    }
    return j;
}

inline json read_arr(const std::string& s, size_t& p) {
    json j(json::value_t::array); p++; skip(s,p);
    if (p < s.size() && s[p] == ']') { p++; return j; }
    for (;;) {
        j.push_back(read_val(s,p));
        skip(s,p);
        if (p < s.size() && s[p] == ']') { p++; break; }
        if (p < s.size() && s[p] == ',') p++;
    }
    return j;
}

inline json read_val(const std::string& s, size_t& p) {
    skip(s,p);
    if (p >= s.size()) return json();
    char c = s[p];
    if (c == '"') return json(read_str(s,p));
    if (c == '{') return read_obj(s,p);
    if (c == '[') return read_arr(s,p);
    if (c == 't') { p+=4; return json(true); }
    if (c == 'f') { p+=5; return json(false); }
    if (c == 'n') { p+=4; return json(); }
    size_t st = p;
    while (p < s.size() && (std::isdigit((unsigned char)s[p]) ||
           s[p]=='.'||s[p]=='-'||s[p]=='+'||s[p]=='e'||s[p]=='E')) p++;
    return json(std::stod(s.substr(st, p-st)));
}

} // namespace detail

inline json json::parse(const std::string& s) {
    size_t p = 0; return detail::read_val(s,p);
}

inline std::string json::dump(int indent, int depth) const {
    std::ostringstream os;
    std::string sp(depth * (indent>=0 ? indent : 0), ' ');
    std::string csp((depth+1) * (indent>=0 ? indent : 0), ' ');
    bool pretty = (indent >= 0);

    switch (type_) {
    case value_t::null:    os << "null"; break;
    case value_t::boolean: os << (bool_ ? "true" : "false"); break;
    case value_t::number:
        if (num_ == std::floor(num_) && std::fabs(num_) < 1e15) {
            os << (long long)num_;
        } else {
            std::ostringstream tmp; tmp << std::setprecision(10) << num_;
            os << tmp.str();
        }
        break;
    case value_t::string:
        os << '"';
        for (char c : str_) {
            if (c=='"') os<<"\\\""; else if (c=='\\') os<<"\\\\";
            else if (c=='\n') os<<"\\n"; else os<<c;
        }
        os << '"'; break;
    case value_t::array:
        if (arr_.empty()) { os << "[]"; break; }
        os << "[";
        if (pretty) os << "\n";
        for (size_t i = 0; i < arr_.size(); i++) {
            if (pretty) os << csp;
            os << arr_[i].dump(indent, depth+1);
            if (i+1 < arr_.size()) os << ",";
            if (pretty) os << "\n";
        }
        if (pretty) os << sp;
        os << "]"; break;
    case value_t::object:
        if (obj_.empty()) { os << "{}"; break; }
        os << "{";
        if (pretty) os << "\n";
        { size_t i = 0;
          for (const auto& kv : obj_) {
            if (pretty) os << csp;
            os << '"' << kv.first << '"' << (pretty ? ": " : ":");
            os << kv.second.dump(indent, depth+1);
            if (++i < obj_.size()) os << ",";
            if (pretty) os << "\n";
          }
        }
        if (pretty) os << sp;
        os << "}"; break;
    }
    return os.str();
}

} // namespace nlohmann
