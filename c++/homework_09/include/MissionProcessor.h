#pragma once
#include "Types.h"
#include <memory>
#include <string>

class ITargetProvider;
class IBallisticSolver;
class IConfigLoader;

class MissionProcessor {
public:
    MissionProcessor(std::unique_ptr<ITargetProvider> t,
                     std::unique_ptr<IBallisticSolver> s,
                     std::unique_ptr<IConfigLoader> l);
    ~MissionProcessor();
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
    double time_;
    Config cfg_;
    AmmoParams ammo_;
};
