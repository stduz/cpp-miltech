#pragma once
#include "Types.h"
#include <string>

class IConfigLoader {
public:
    virtual bool load(const std::string& path) = 0;
    virtual Config getConfig() = 0;
    virtual AmmoParams getAmmoParams() = 0;
    virtual ~IConfigLoader() = default;
};
