#include "configreader.hpp"
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <strings.h>

using namespace std;

typedef map<string, map<string, string>*> STR_MAP;
typedef STR_MAP::iterator STR_MAP_ITER;

class ConfigReadImpl : public ConfigReader {
public:
    ConfigReadImpl() {}

    virtual ~ConfigReadImpl() {
        for (STR_MAP_ITER it = _map.begin(); it != _map.end(); ++it) {
            delete it->second;
        }
    }

    virtual bool Load(const string& file) {
        ifstream ifs(file.c_str());
        if (!ifs.good()) return false;

        string line;
        map<string, string> *m = NULL;

        while (!ifs.eof() && ifs.good()) {
            getline(ifs, line);
            string section;

            if (isSection(line, section)) {
                // cout << '[' << section << ']' << endl;
                STR_MAP_ITER it = _map.find(section);
                if (it == _map.end()) {
                    m = new map<string, string>();
                    _map.insert(STR_MAP::value_type(section, m));
                } else {
                    m = it->second;
                }
                continue;
            }

            size_t equ_pos = line.find('=');
            if (equ_pos == string::npos) continue;
            string key = line.substr(0, equ_pos);
            string value = line.substr(equ_pos + 1);
            key = trim(key);
            value = trim(value);

            if (key.empty()) continue;

            if (key[0] == '#' || key[0] == ';')  // skip comment
                continue;

            // cout << key << " = " << value << endl;
            map<string, string>::iterator it1 = m->find(key);
            if (it1 != m->end()) {
                it1->second = value;
            } else {
                m->insert(map<string, string>::value_type(key, value));
            }
        }

        ifs.close();
        return true;
    }

    virtual string GetString(const string& section,
                             const string& key,
                             const string& default_value) {
        STR_MAP_ITER it = _map.find(section);
        if (it != _map.end()) {
            map<string, string>::const_iterator it1 = it->second->find(key);
            if (it1 != it->second->end()) {
                return it1->second;
            }
        }
        return default_value;
    }

    virtual vector<string> GetStringList(const string& section,
                                         const string& key) {
        vector<string> v;
        string str = GetString(section, key, "");
        string sep = ", \t";
        string substr;
        string::size_type start = 0;
        string::size_type index;

        while ((index = str.find_first_of(sep, start)) != string::npos) {
            substr = str.substr(start, index - start);
            v.push_back(substr);

            start = str.find_first_not_of(sep, index);
            if (start == string::npos)
                return v;
        }

        substr = str.substr(start);
        v.push_back(substr);

        return v;
    }

    virtual unsigned GetNumber(const string& section,
                                const string& key,
                                unsigned default_value) {
        STR_MAP_ITER it = _map.find(section);
        if (it != _map.end()) {
            map<string, string>::const_iterator it1 = it->second->find(key);
            if (it1 != it->second->end()) {
                return parseNumber(it1->second);
            }
        }
        return default_value;
    }

    virtual bool GetBool(const string& section,
                         const string& key,
                         bool default_value) {
        STR_MAP_ITER it = _map.find(section);
        if (it != _map.end()) {
            map<string, string>::const_iterator it1 = it->second->find(key);
            if (it1 != it->second->end()) {
                if (strcasecmp(it1->second.c_str(), "true") == 0) {
                    return true;
                }
            }
        }
        return default_value;
    }

private:

    /**
     * determine whether a line contains a section name
     *
     * @param line    a line from the config file
     * @param section section description
     *
     * @return return true and set section if the line is a section, else false
     */
    bool isSection(string line, string& section) {
        section = trim(line);

        if (section.empty() || section.length() <= 2)
            return false;

        if (section.at(0) != '[' || section.at(section.length() - 1) != ']')
            return false;

        section = section.substr(1, section.length() - 2);
        section = trim(section);

        return true;
    }

    unsigned parseNumber(const string& s) {
        istringstream iss(s);
        long long v = 0;
        iss >> v;
        return v;
    }

    string trimLeft(const string& s) {
        size_t first_not_empty = 0;

        string::const_iterator beg = s.begin();
        while (beg != s.end()) {
            if (!isspace(*beg)) break;
            first_not_empty++;
            beg++;
        }

        return s.substr(first_not_empty);
    }

    string trimRight(const string& s) {
        size_t last_not_empty = s.length();

        string::const_iterator end = s.end();
        while (end != s.begin()) {
            end--;
            if (!isspace(*end)) break;
            last_not_empty--;
        }
        return s.substr(0, last_not_empty);
    }

    /**
     * remove space characters before and after a string
     *
     * @param s string
     *
     * @return return a string after trimed
     */
    string trim(const string& s) {
        return trimLeft(trimRight(s));
    }

private:
    STR_MAP _map;
};

ConfigReader& ConfigReader::getConfigReader() {
    static ConfigReadImpl impl;
    return impl;
}
