#include "providers/ThreadSafeTargetProvider.h"
#include "json.hpp"
#include <fstream>
#include <chrono>
#include <cmath>

ThreadSafeTargetProvider::~ThreadSafeTargetProvider() { stop(); }

bool ThreadSafeTargetProvider::load(const std::string& path) {
    init_.clear();
    std::ifstream f(path);
    if (!f) return false;
    nlohmann::json j;
    f >> j;
    init_.reserve(j.size());
    for (size_t i = 0; i < j.size(); i++) {
        InitTarget t{};
        t.pos.x = j[i]["x"].get<double>();
        t.pos.y = j[i]["y"].get<double>();
        t.pos.z = 0.0;
        t.vel.x = j[i].value("vx", 0.0);
        t.vel.y = j[i].value("vy", 0.0);
        t.vel.z = 0.0;
        init_.push_back(t);
    }
    std::lock_guard<std::mutex> lk(mu_);
    current_.resize(init_.size());
    for (size_t i = 0; i < init_.size(); i++) {
        current_[i].pos = init_[i].pos;
        current_[i].vel = init_[i].vel;
    }
    simTime_ = 0.0;
    return !init_.empty();
}

void ThreadSafeTargetProvider::start(double timeStep, double timeScale) {
    if (running_.load()) return;
    running_ = true;
    started_ = true;
    thread_ = std::thread(&ThreadSafeTargetProvider::run, this, timeStep, timeScale);
}

void ThreadSafeTargetProvider::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void ThreadSafeTargetProvider::run(double timeStep, double timeScale) {
    using clock = std::chrono::steady_clock;
    auto realStep = std::chrono::duration<double>(timeStep / timeScale);
    auto next = clock::now() + realStep;
    while (running_.load()) {
        std::this_thread::sleep_until(next);
        next += realStep;
        {
            std::lock_guard<std::mutex> lk(mu_);
            simTime_ += timeStep;
            for (size_t i = 0; i < current_.size(); i++) {
                current_[i].pos.x = init_[i].pos.x + init_[i].vel.x * simTime_;
                current_[i].pos.y = init_[i].pos.y + init_[i].vel.y * simTime_;
                current_[i].vel = init_[i].vel;
            }
        }
    }
}

int ThreadSafeTargetProvider::getTargetCount() {
    std::lock_guard<std::mutex> lk(mu_);
    return static_cast<int>(current_.size());
}

Target ThreadSafeTargetProvider::getTarget(int idx) {
    std::lock_guard<std::mutex> lk(mu_);
    return current_[static_cast<size_t>(idx)];
}
