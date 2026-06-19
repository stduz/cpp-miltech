#pragma once
#include "Types.h"
#include <memory>
#include <string>
#include <atomic>
#include <thread>

class ITargetProvider;
class IBallisticSolver;
class IConfigLoader;
class IDronePhysics;

class MissionProcessor {
public:
    MissionProcessor(std::unique_ptr<IBallisticSolver> s,
                     std::unique_ptr<IConfigLoader> l,
                     ITargetProvider* targets,
                     IDronePhysics* physics);
    ~MissionProcessor();
    bool init(const std::string& path);
    void start();
    void stop();

private:
    std::unique_ptr<IBallisticSolver> solver_;
    std::unique_ptr<IConfigLoader> loader_;
    ITargetProvider* targets_;
    IDronePhysics* physics_;
    Config cfg_;
    AmmoParams ammo_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    double simTime_{0.0};

    void run();
};
