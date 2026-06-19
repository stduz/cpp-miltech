#include "physics/DronePhysics.h"
#include <chrono>
#include <cmath>
#include <numbers>

DronePhysics::~DronePhysics() { stop(); }

void DronePhysics::init(const Vec3& pos, double speed, double heading) {
    std::lock_guard<std::mutex> lk(mu_);
    pos_ = pos;
    speed_ = speed;
    heading_ = heading;
    timestamp_ = 0.0;
}

void DronePhysics::start(double timeStep, double timeScale) {
    if (running_.load()) return;
    running_ = true;
    thread_ = std::thread(&DronePhysics::run, this, timeStep, timeScale);
}

void DronePhysics::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void DronePhysics::run(double timeStep, double timeScale) {
    using clock = std::chrono::steady_clock;
    auto realStep = std::chrono::duration<double>(timeStep / timeScale);
    auto next = clock::now() + realStep;
    while (running_.load()) {
        std::this_thread::sleep_until(next);
        next += realStep;
        std::lock_guard<std::mutex> lk(mu_);
        double rad = heading_ * std::numbers::pi / 180.0;
        pos_.x += speed_ * std::cos(rad) * timeStep;
        pos_.y += speed_ * std::sin(rad) * timeStep;
        timestamp_ += timeStep;
    }
}

DroneTelemetry DronePhysics::getTelemetry() const {
    std::lock_guard<std::mutex> lk(mu_);
    return {pos_, speed_, heading_, timestamp_};
}
