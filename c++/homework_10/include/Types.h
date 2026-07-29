#pragma once
#include <string>

struct Vec3 { double x, y, z; };
struct Target { Vec3 pos; Vec3 vel; };
struct DropPoint { bool ok; double x, y; bool has_maneuver; double man_x, man_y; };
struct AmmoParams { double mass, drag, lift; };

struct Config {
    Vec3 drone;
    double speed, apath;
    std::string ammo;
    double targetTimeStep;
    double physicsTimeStep;
    double simTimeStep;
    double timeScale;
};

struct DroneTelemetry {
    Vec3 pos;
    double speed;
    double heading;
    double timestamp;
};
