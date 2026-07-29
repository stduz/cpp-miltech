#include "MissionProcessor.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/IConfigLoader.h"

MissionProcessor::MissionProcessor(std::unique_ptr<ITargetProvider> t,
                                   std::unique_ptr<IBallisticSolver> s,
                                   std::unique_ptr<IConfigLoader> l)
    : targets_(std::move(t)), solver_(std::move(s)), loader_(std::move(l)),
      idx_(0), time_(0.0), cfg_{}, ammo_{} {}

MissionProcessor::~MissionProcessor() = default;

bool MissionProcessor::init(const std::string& path) {
    if (!loader_->load(path)) return false;
    cfg_ = loader_->getConfig();
    ammo_ = loader_->getAmmoParams();
    idx_ = 0;
    time_ = 0.0;
    return true;
}

bool MissionProcessor::hasNext() {
    return idx_ < targets_->getTargetCount();
}

DropPoint MissionProcessor::step() {
    Target tgt = targets_->getTarget(idx_++);
    DropPoint dp = solver_->solve(cfg_.drone, tgt.pos, cfg_.speed, cfg_.apath, ammo_);
    time_ += cfg_.apath;
    return dp;
}

void MissionProcessor::reset() { idx_ = 0; time_ = 0.0; }

void MissionProcessor::changeSolver(std::unique_ptr<IBallisticSolver> s) {
    solver_ = std::move(s);
}
