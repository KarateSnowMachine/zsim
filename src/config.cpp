/** $lic$
 * Copyright (C) 2012-2015 by Massachusetts Institute of Technology
 * Copyright (C) 2010-2013 by The Board of Trustees of Stanford University
 *
 * This file is part of zsim.
 *
 * zsim is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2.
 *
 * If you use this software in your research, we request that you reference
 * the zsim paper ("ZSim: Fast and Accurate Microarchitectural Simulation of
 * Thousand-Core Systems", Sanchez and Kozyrakis, ISCA-40, June 2013) as the
 * source of the simulator in any publications that use this software, and that
 * you send us a citation of your work.
 *
 * zsim is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <typeinfo>
#include <vector>
#include "log.h"

using json = nlohmann::json;
using std::string;
using std::stringstream;
using std::vector;

// Restrict use of long long, which libconfig uses as its int64
typedef long long lc_int64;  // NOLINT(runtime/int)

// TODO(prosenfeld): move to a utils file or something 
std::vector<string> split(std::string input, std::string chars)
{
    using std::string; 
    using std::vector;

    vector<string> ret;

    string cur = input;

    size_t pos = 0;

    while (!cur.empty())
    {
        pos = cur.find_first_of(chars);
        string tmp = cur.substr(0, pos);
        ret.push_back(tmp);


        if (pos == string::npos) {
            cur.erase();
        } else {
            cur.erase(0, pos +1);
        }
    }

    // If not at npos, then there is still an extra empty string at the end.
    if (pos != string::npos)
    {
        ret.push_back("");
    }

    return ret;
}

Config::Config(const char* inFile) {
    std::ifstream is{inFile};
    is >> inJson;
}

// Helper function: Add "*"-prefixed vars, which are used by our scripts but not zsim, to outCfg
// Returns number of copied vars
//
// TODO(prosenfeld): we don't use this functionality, so just leave this
// function unimplemented; it's reasonably straightforward to implement but
// I don't care to spend the time on it now 
static uint32_t copyNonSimVars(json tree1, json tree2, std::string prefix) {
    warn("Unimplemented optional functionality to copy non sim vars to output config");
#if 0
    uint32_t copied = 0;
    for (uint32_t i = 0; i < (uint32_t)s1.getLength(); i++) {
        const char* name = s1[i].getName();
        if (name[0] == '*') {
            if (s2.exists(name)) panic("Setting %s was read, should be private", (prefix + name).c_str());
            // This could be as simple as:
            //s2.add(s1[i].getType()) = s1[i];
            // However, because Setting kinda sucks, we need to go type by type:
            libconfig::Setting& ns = s2.add(name, s1[i].getType());
            if      (libconfig::Setting::Type::TypeInt     == s1[i].getType()) ns = (int) s1[i];
            else if (libconfig::Setting::Type::TypeInt64   == s1[i].getType()) ns = (lc_int64) s1[i];
            else if (libconfig::Setting::Type::TypeBoolean == s1[i].getType()) ns = (bool) s1[i];
            else if (libconfig::Setting::Type::TypeString  == s1[i].getType()) ns = (const char*) s1[i];
            else panic("Unknown type for priv setting %s, cannot copy", (prefix + name).c_str());
            copied++;
        }

        if (s1[i].isGroup() && s2.exists(name)) {
            copied += copyNonSimVars(s1[i], s2[name], prefix + name + ".");
        }
    }
    return copied;
#endif
    return 0;
}

// Helper function: Compares two settings recursively, checking for inclusion
// Returns number of settings without inclusion (given but unused)
//
// TODO(prosenfeld): this is a nice check to do but don't bother implementing
// it for now
static uint32_t checkIncluded(json tree1, json tree2, std::string prefix) {
    warn("Unimplemented optional functionality to count unused config items");
    uint32_t unused = 0;
#if 0
    for (uint32_t i = 0; i < (uint32_t)s1.getLength(); i++) {
        const char* name = s1[i].getName();
        if (!s2.exists(name)) {
            warn("Setting %s not used during configuration", (prefix + name).c_str());
            unused++;
        } else if (s1[i].isGroup()) {
            unused += checkIncluded(s1[i], s2[name], prefix + name + ".");
        }
    }
#endif
    return unused;
}


//Called when initialization ends. Writes output config, and emits warnings for unused input settings
void Config::writeAndClose(const char* outFile, bool strictCheck) {
    uint32_t nonSimVars = copyNonSimVars(inJson, outJson, std::string(""));
    uint32_t unused = checkIncluded(inJson, outJson, std::string(""));

    if (nonSimVars) info("Copied %d non-sim var%s to output config", nonSimVars, (nonSimVars > 1)? "s" : "");
    if (unused) {
        if (strictCheck) {
            panic("%d setting%s not used during configuration", unused, (unused > 1)? "s" : "");
        } else {
            warn("%d setting%s not used during configuration", unused, (unused > 1)? "s" : "");
        }
    }
    // TODO(prosenfeld): check if << can throw
    std::ofstream os{outFile};
    os << outJson.dump(4);
}


bool Config::exists(const char* key) {
    auto tmp_root = inJson;
    auto pieces = split(std::string(key), ".");
    for (const auto &piece : pieces) {
        if (tmp_root.contains(piece)) {
            tmp_root = inJson[piece];
        } else {
            return false;
        }
    }
    return true;
}


template<typename T> static bool getEq(T v1, T v2);
template<> bool getEq<int>(int v1, int v2) {return v1 == v2;}
template<> bool getEq<lc_int64>(lc_int64 v1, lc_int64 v2) {return v1 == v2;}
template<> bool getEq<bool>(bool v1, bool v2) {return v1 == v2;}
template<> bool getEq<const char*>(const char* v1, const char* v2) {return strcmp(v1, v2) == 0;}
template<> bool getEq<double>(double v1, double v2) {return v1 == v2;}

string join(vector<string>::const_iterator start, vector<string>::const_iterator stop, const string &glue) {
    string output;
    for (vector<string>::const_iterator it = start; it != stop; ++it) {
        if (!glue.empty() && it != start) {
            output += glue;
        }
        output += *it;
    }
    return output;
}

template<typename T> static void writeVar(json *curr_node, const char* key, T val) {
    auto pieces = split(key, ".");
    auto this_key = pieces[0];
    auto leaf = pieces.back();

    // Base case - only one piece left in the path so just set the value 
    if (pieces.size() == 1) {
        (*curr_node)[leaf] = val;
        return;
    }

    // Special case - middle of a path through the tree needs to be created
    if (curr_node->is_null()) {
        (*curr_node)[this_key] = json();
    }

    // Recursive case -- continue down the tree 
    auto next_key = join(pieces.begin()+1, pieces.end(), string("."));
    writeVar(&(*curr_node)[this_key], next_key.c_str(), val);
}

template<typename T>
T Config::genericGet(const char* key, T def) {
    T val;
    auto tmp_root = inJson;
    auto pieces = split(std::string(key), ".");
    for (const auto &piece : pieces) {
        if (tmp_root.contains(piece)) {
            tmp_root = tmp_root[piece];
        } else {
            val = def;
            goto out;
        }
    }
    val = tmp_root.get<T>();
out:
    writeVar(&outJson, key, val);
    return val;
}

template<typename T>
T Config::genericGet(const char* key) {
    return genericGet<T>(key, T());
}

// TODO(prosenfeld): I think these specializations are unnecessary now -- it
// seems like they were primarily here to do type conversions from libconfig
// but I don't think we need that with the JSON library

//Template specializations for access interface
template<> uint32_t Config::get<uint32_t>(const char* key) {return (uint32_t) genericGet<int>(key);}
template<> uint64_t Config::get<uint64_t>(const char* key) {return (uint64_t) genericGet<lc_int64>(key);}
template<> bool Config::get<bool>(const char* key) {return genericGet<bool>(key);}
template<> std::string Config::get<std::string>(const char* key) {return genericGet<std::string>(key);}
template<> double Config::get<double>(const char* key) {return (double) genericGet<double>(key);}
template<> const char* Config::get<const char *>(const char* key) {return genericGet<std::string>(key).c_str();}

template<> uint32_t Config::get<uint32_t>(const char* key, uint32_t def) {return (uint32_t) genericGet<int>(key, (int)def);}
template<> uint64_t Config::get<uint64_t>(const char* key, uint64_t def) {return (uint64_t) genericGet<lc_int64>(key, (lc_int64)def);}
template<> bool Config::get<bool>(const char* key, bool def) {return genericGet<bool>(key, def);}
template<> std::string Config::get<std::string>(const char* key, std::string def) {return genericGet<std::string>(key, def);}
template<> double Config::get<double>(const char* key, double def) {return (double) genericGet<double>(key, (double)def);}
template<> const char* Config::get<const char *>(const char* key, const char *def) {return genericGet<std::string>(key, std::string(def)).c_str();}


//Get subgroups in a specific key
void Config::subgroups(const char* key, std::vector<const char*>& grps) {
    // TODO(prosenfeld): this probably needs a lot of error handling code
    auto json_tmp = inJson;
    auto pieces = split(std::string(key), ".");
    for (const auto &piece : pieces) {
        json_tmp = json_tmp[piece];
    }
    if (json_tmp.is_object()) {
        for (const auto &elem : json_tmp.items()) {
            grps.push_back(elem.key().c_str());
        }
    }
}


/* Config value parsing functions */

//Range parsing, for process masks

//Helper, from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void Tokenize(const string& str, vector<string>& tokens, const string& delimiters) {
    // Skip delimiters at beginning.
    string::size_type lastPos = 0; //dsm: DON'T //str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

struct Range {
    int32_t min;
    int32_t sup;
    int32_t step;

    explicit Range(string r)  {
        vector<string> t;
        Tokenize(r, t, ":");
        vector<uint32_t> n;
        for (auto s : t) {
            stringstream ss(s);
            uint32_t x = 0;
            ss >> x;
            if (ss.fail()) panic("%s in range %s is not a valid number", s.c_str(), r.c_str());
            n.push_back(x);
        }
        switch (n.size()) {
            case 1:
                min = n[0];
                sup = min + 1;
                step = 1;
                break;
            case 2:
                min = n[0];
                sup = n[1];
                step = 1;
                break;
            case 3:
                min = n[0];
                sup = n[1];
                step = n[2];
                break;
            default:
                panic("Range '%s' can only have 1-3 numbers delimited by ':', %ld parsed", r.c_str(), n.size());
        }

        //Final error-checking
        if (min < 0 || step < 0 || sup < 0) panic("Range %s has negative numbers", r.c_str());
        if (step == 0) panic("Range %s has 0 step!", r.c_str());
        if (min >= sup) panic("Range %s has min >= sup!", r.c_str());
    }

    void fill(vector<bool>& mask) {
        for (int32_t i = min; i < sup; i += step) {
            if (i >= (int32_t)mask.size() || i < 0) panic("Range %d:%d:%d includes out-of-bounds %d (mask limit %ld)", min, step, sup, i, mask.size()-1);
            mask[i] = true;
        }
    }
};

std::vector<bool> ParseMask(const std::string& maskStr, uint32_t maskSize) {
    vector<bool> mask;
    mask.resize(maskSize);

    vector<string> ranges;
    Tokenize(maskStr, ranges, " ");
    for (auto r : ranges) {
        if (r.length() == 0) continue;
        Range range(r);
        range.fill(mask);
    }
    return mask;
}

//List parsing
template <typename T>
std::vector<T> ParseList(const std::string& listStr, const char* delimiters) {
    vector<string> nums;
    Tokenize(listStr, nums, delimiters);

    vector<T> res;
    for (auto n : nums) {
        if (n.length() == 0) continue;
        stringstream ss(n);
        T x;
        ss >> x;
        if (ss.fail()) panic("%s in list [%s] could not be parsed", n.c_str(), listStr.c_str());
        res.push_back(x);
    }
    return res;
}

//Instantiations
template std::vector<uint32_t> ParseList<uint32_t>(const std::string& listStr, const char* delimiters);
template std::vector<uint64_t> ParseList<uint64_t>(const std::string& listStr, const char* delimiters);
template std::vector<std::string> ParseList(const std::string& listStr, const char* delimiters);
