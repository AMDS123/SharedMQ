#ifndef __ConfigReader__
#define __ConfigReader__

#include <string>
#include <vector>

class ConfigReader {
public:
    virtual ~ConfigReader() {};

    virtual bool Load(const std::string& file) = 0;

    virtual std::string GetString(const std::string& section,
                                  const std::string& key,
                                  const std::string& default_value = "") = 0;

    virtual std::vector<std::string> GetStringList(const std::string& section,
                                                   const std::string& key) = 0;

    virtual unsigned GetNumber(const std::string& section,
                                const std::string& key,
                                unsigned default_value = 0) = 0;

    virtual bool GetBool(const std::string& section,
                         const std::string& key,
                         bool default_value = false) = 0;

    static ConfigReader& getConfigReader();
};

#endif
