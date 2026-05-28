#include "config/FileConfigLoader.h"
#include <fstream>

const std::unordered_map<std::string, AmmoParams> FileConfigLoader::AMMO_TABLE = {
    {"VOG-17",      {0.35, 0.07, 0.0}},
    {"M67",         {0.60, 0.10, 0.0}},
    {"RKG-3",       {1.20, 0.10, 0.0}},
    {"GLIDING-VOG", {0.45, 0.10, 1.0}},
    {"GLIDING-RKG", {1.40, 0.10, 1.0}},
};

bool FileConfigLoader::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    f >> cfg_.drone.x >> cfg_.drone.y >> cfg_.drone.z
      >> cfg_.speed >> cfg_.apath >> cfg_.ammo;
    if (f.fail()) return false;
    auto it = AMMO_TABLE.find(cfg_.ammo);
    if (it == AMMO_TABLE.end()) return false;
    ammo_ = it->second;
    return true;
}

Config FileConfigLoader::getConfig() { return cfg_; }
AmmoParams FileConfigLoader::getAmmoParams() { return ammo_; }
