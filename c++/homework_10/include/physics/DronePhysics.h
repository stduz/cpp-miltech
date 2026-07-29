#pragma once
#include "interfaces/IDronePhysics.h"
#include <mutex>
#include <thread>
#include <atomic>

class DronePhysics : public IDronePhysics {
public:
    ~DronePhysics();
    void init(const Vec3& pos, double speed, double heading);
    void start(double timeStep, double timeScale);
    void stop();
    DroneTelemetry getTelemetry() const override;

private:
    mutable std::mutex mu_;
    Vec3 pos_{};
    double speed_{0.0};
    double heading_{0.0};
    double timestamp_{0.0};
    std::atomic<bool> running_{false};
    std::thread thread_;

    void run(double timeStep, double timeScale);
};
