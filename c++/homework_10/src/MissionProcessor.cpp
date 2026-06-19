#include "MissionProcessor.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/IConfigLoader.h"
#include "interfaces/IDronePhysics.h"
#include <chrono>
#include <cstdio>

MissionProcessor::MissionProcessor(std::unique_ptr<IBallisticSolver> s,
                                   std::unique_ptr<IConfigLoader> l,
                                   ITargetProvider* targets,
                                   IDronePhysics* physics)
    : solver_(std::move(s)), loader_(std::move(l)),
      targets_(targets), physics_(physics) {}

MissionProcessor::~MissionProcessor() { stop(); }

bool MissionProcessor::init(const std::string& path) {
    if (!loader_->load(path)) return false;
    cfg_ = loader_->getConfig();
    ammo_ = loader_->getAmmoParams();
    simTime_ = 0.0;
    return true;
}

void MissionProcessor::start() {
    if (running_.load()) return;
    running_ = true;
    thread_ = std::thread(&MissionProcessor::run, this);
}

void MissionProcessor::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void MissionProcessor::run() {
    using clock = std::chrono::steady_clock;
    auto realStep = std::chrono::duration<double>(cfg_.simTimeStep / cfg_.timeScale);
    auto next = clock::now() + realStep;

    while (running_.load()) {
        std::this_thread::sleep_until(next);
        next += realStep;

        DroneTelemetry tel = physics_->getTelemetry();
        int n = targets_->getTargetCount();

        printf("T=%.3f\n", tel.timestamp);
        for (int i = 0; i < n; i++) {
            Target tgt = targets_->getTarget(i);
            DropPoint dp = solver_->solve(tel.pos, tgt.pos,
                                          cfg_.speed, cfg_.apath, ammo_);
            if (!dp.ok) {
                printf("INVALID\n");
                continue;
            }
            if (dp.has_maneuver)
                printf("FIRE %.2f %.2f MANEUVER %.2f %.2f\n",
                       dp.x, dp.y, dp.man_x, dp.man_y);
            else
                printf("FIRE %.2f %.2f\n", dp.x, dp.y);
        }
        simTime_ += cfg_.simTimeStep;
    }
}
