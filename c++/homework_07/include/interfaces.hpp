#pragma once

struct Vec3 { double x, y, z; };
struct Target { Vec3 pos; };
struct DropPoint { bool ok; double x, y; bool has_maneuver; double man_x, man_y; };
struct AmmoParams { double mass, drag, lift; };
struct Config { Vec3 drone; double speed, apath; char ammo[32]; };

class ITargetProvider {
public:
    virtual int getTargetCount() = 0;
    virtual Target getTarget(int idx) = 0;
    virtual ~ITargetProvider() = default;
};

class IBallisticSolver {
public:
    virtual DropPoint solve(const Vec3& drone, const Vec3& tgt,
                            double speed, double apath,
                            const AmmoParams& p) = 0;
    virtual ~IBallisticSolver() = default;
};

class IConfigLoader {
public:
    virtual bool load(const char* path) = 0;
    virtual Config getConfig() = 0;
    virtual AmmoParams getAmmoParams() = 0;
    virtual ~IConfigLoader() = default;
};
