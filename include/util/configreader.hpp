#ifndef __ConfigReader__
#define __ConfigReader__

#include <string>
#include <vector>
#include <map>

namespace util
{

typedef std::map<std::string, std::map<std::string, std::string>*> STR_MAP;
typedef STR_MAP::iterator STR_MAP_ITER;

class ConfigReader {
public:
    ~ConfigReader();
    std::string GetString(const std::string& section, const std::string& key, const std::string& default_value = "");
    std::vector<std::string> GetStringList(const std::string& section, const std::string& key);
    unsigned GetNumber(const std::string& section, const std::string& key, unsigned default_value = 0);
    bool GetBool(const std::string& section, const std::string& key, bool default_value = false);
    
    static ConfigReader *getConfigReader(const char *file);

private:
    ConfigReader()
    {
    }

    bool isSection(std::string line, std::string& section);
    unsigned parseNumber(const std::string& s);
    std::string trimLeft(const std::string& s);
    std::string trimRight(const std::string& s);
    std::string trim(const std::string& s);
    bool Load(const char *file);

    static ConfigReader* config;

    STR_MAP _map;
};

}

#endif