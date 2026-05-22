#pragma once
#include "interfaces.hpp"

class MissionProcessor {
public:
    MissionProcessor(ITargetProvider* t, IBallisticSolver* s, IConfigLoader* l);
    bool init(const char* path);
    bool hasNext();
    DropPoint step();
    void reset();
    void changeSolver(IBallisticSolver* s);
private:
    ITargetProvider* targets_;
    IBallisticSolver* solver_;
    IConfigLoader* loader_;
    int idx_;
    Config cfg_;
    AmmoParams ammo_;
};
