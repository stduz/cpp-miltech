#pragma once
#include "interfaces/ITargetProvider.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/IConfigLoader.h"
#include <memory>
#include <string>

class MissionProcessor {
public:
    MissionProcessor(std::unique_ptr<ITargetProvider> t,
                     std::unique_ptr<IBallisticSolver> s,
                     std::unique_ptr<IConfigLoader> l);
    bool init(const std::string& path);
    bool hasNext();
    DropPoint step();
    void reset();
    void changeSolver(std::unique_ptr<IBallisticSolver> s);
private:
    std::unique_ptr<ITargetProvider> targets_;
    std::unique_ptr<IBallisticSolver> solver_;
    std::unique_ptr<IConfigLoader> loader_;
    int idx_;
    Config cfg_;
    AmmoParams ammo_;
};
