#pragma once
#include <string>

struct Vec3 { double x, y, z; };
struct Target { Vec3 pos; };
struct DropPoint { bool ok; double x, y; bool has_maneuver; double man_x, man_y; };
struct AmmoParams { double mass, drag, lift; };
struct Config { Vec3 drone; double speed, apath; std::string ammo; };
