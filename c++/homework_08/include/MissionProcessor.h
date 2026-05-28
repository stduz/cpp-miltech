#pragma once
#include "interfaces/ITargetProvider.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/IConfigLoader.h"
#include <string>

class MissionProcessor {
public:
    MissionProcessor(ITargetProvider* t, IBallisticSolver* s, IConfigLoader* l);
    bool init(const std::string& path);
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
