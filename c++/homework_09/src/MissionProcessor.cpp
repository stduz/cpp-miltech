#include "MissionProcessor.h"

MissionProcessor::MissionProcessor(std::unique_ptr<ITargetProvider> t,
                                   std::unique_ptr<IBallisticSolver> s,
                                   std::unique_ptr<IConfigLoader> l)
    : targets_(std::move(t)), solver_(std::move(s)), loader_(std::move(l)),
      idx_(0), cfg_{}, ammo_{} {}

bool MissionProcessor::init(const std::string& path) {
    if (!loader_->load(path)) return false;
    cfg_ = loader_->getConfig();
    ammo_ = loader_->getAmmoParams();
    idx_ = 0;
    return true;
}

bool MissionProcessor::hasNext() {
    return idx_ < targets_->getTargetCount();
}

DropPoint MissionProcessor::step() {
    Target tgt = targets_->getTarget(idx_++);
    return solver_->solve(cfg_.drone, tgt.pos, cfg_.speed, cfg_.apath, ammo_);
}

void MissionProcessor::reset() { idx_ = 0; }

void MissionProcessor::changeSolver(std::unique_ptr<IBallisticSolver> s) {
    solver_ = std::move(s);
}
