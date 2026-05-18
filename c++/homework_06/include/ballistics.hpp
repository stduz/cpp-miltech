#pragma once

struct BInput {
    double x, y, z;
    double tx, ty;
    double speed;
    double apath;
    const char* ammo;
};

struct BResult {
    bool ok;
    double fire_x, fire_y;
    bool has_maneuver;
    double man_x, man_y;
};

BResult compute(const BInput& in);
