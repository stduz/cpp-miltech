#pragma once
#include "interfaces/ITargetProvider.h"
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

class ThreadSafeTargetProvider : public ITargetProvider {
public:
    ~ThreadSafeTargetProvider();
    bool load(const std::string& path);
    void start(double timeStep, double timeScale);
    void stop();
    int getTargetCount() override;
    Target getTarget(int idx) override;

private:
    struct InitTarget { Vec3 pos; Vec3 vel; };
    std::vector<InitTarget> init_;
    std::vector<Target> current_;
    mutable std::mutex mu_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    double simTime_{0.0};

    void run(double timeStep, double timeScale);
};
