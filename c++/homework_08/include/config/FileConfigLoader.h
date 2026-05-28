#pragma once
#include "interfaces/IConfigLoader.h"
#include <unordered_map>
#include <string>

class FileConfigLoader : public IConfigLoader {
public:
    bool load(const std::string& path) override;
    Config getConfig() override;
    AmmoParams getAmmoParams() override;
private:
    Config cfg_{};
    AmmoParams ammo_{};
    static const std::unordered_map<std::string, AmmoParams> AMMO_TABLE;
};
