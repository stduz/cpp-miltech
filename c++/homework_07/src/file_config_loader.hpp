#pragma once
#include "interfaces.hpp"

class FileConfigLoader : public IConfigLoader {
public:
    bool load(const char* path) override;
    Config getConfig() override;
    AmmoParams getAmmoParams() override;
private:
    Config cfg_{};
    AmmoParams ammo_{};
    bool loaded_ = false;
    static AmmoParams findAmmo(const char* name);
};
