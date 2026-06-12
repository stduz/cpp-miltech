#include "drone/DroneController.h"
#include "drone/DroneStates.h"

DroneController::DroneController(DroneContext ctx)
    : ctx_(ctx), state_(std::make_unique<StateStopped>()) {}

void DroneController::update() {
    auto next = state_->execute(ctx_);
    if (next) state_ = std::move(next);
}

const DroneContext& DroneController::context() const { return ctx_; }
