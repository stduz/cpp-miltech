#include "mission.hpp"

MissionProcessor::MissionProcessor(ITargetProvider* t, IBallisticSolver* s, IConfigLoader* l)
    : targets_(t), solver_(s), loader_(l), idx_(0), cfg_{}, ammo_{} {}

bool MissionProcessor::init(const char* path) {
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
    Target tgt = targets_->getTarget(idx_);
    idx_++;
    return solver_->solve(cfg_.drone, tgt.pos, cfg_.speed, cfg_.apath, ammo_);
}

void MissionProcessor::reset() { idx_ = 0; }

void MissionProcessor::changeSolver(IBallisticSolver* s) { solver_ = s; }
