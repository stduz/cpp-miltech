#include "file_config_loader.hpp"
#include <cstdio>
#include <cstring>

static const int NAMMO = 5;
static const char NAMES[NAMMO][15] = {
    "VOG-17", "M67", "RKG-3", "GLIDING-VOG", "GLIDING-RKG"};
static const double MASS[NAMMO] = {0.35, 0.60, 1.20, 0.45, 1.40};
static const double DRAG[NAMMO] = {0.07, 0.10, 0.10, 0.10, 0.10};
static const double LIFT[NAMMO] = {0.0, 0.0, 0.0, 1.0, 1.0};

AmmoParams FileConfigLoader::findAmmo(const char* name) {
    for (int i = 0; i < NAMMO; i++)
        if (strcmp(name, NAMES[i]) == 0)
            return {MASS[i], DRAG[i], LIFT[i]};
    return {};
}

bool FileConfigLoader::load(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    int ok = fscanf(f, "%lf %lf %lf %lf %lf %31s",
                    &cfg_.drone.x, &cfg_.drone.y, &cfg_.drone.z,
                    &cfg_.speed, &cfg_.apath, cfg_.ammo);
    fclose(f);
    if (ok != 6) return false;
    ammo_ = findAmmo(cfg_.ammo);
    loaded_ = true;
    return true;
}

Config FileConfigLoader::getConfig() { return cfg_; }
AmmoParams FileConfigLoader::getAmmoParams() { return ammo_; }
