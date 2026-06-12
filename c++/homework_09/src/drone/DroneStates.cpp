#include "drone/DroneStates.h"
#include <cmath>

static const double ACCEL_STEP = 1.0;
static const double TURN_STEP = 5.0;

std::unique_ptr<IDroneState> StateStopped::execute(DroneContext& ctx) {
    if (ctx.target_speed > 0.0)
        return std::make_unique<StateAccelerating>();
    return nullptr;
}

std::unique_ptr<IDroneState> StateAccelerating::execute(DroneContext& ctx) {
    ctx.speed += ACCEL_STEP;
    if (ctx.speed >= ctx.target_speed) {
        ctx.speed = ctx.target_speed;
        return std::make_unique<StateMoving>();
    }
    return nullptr;
}

std::unique_ptr<IDroneState> StateMoving::execute(DroneContext& ctx) {
    if (ctx.stop_requested)
        return std::make_unique<StateDecelerating>();
    double diff = ctx.target_heading - ctx.heading;
    if (std::fabs(diff) > 0.01)
        return std::make_unique<StateTurning>();
    return nullptr;
}

std::unique_ptr<IDroneState> StateTurning::execute(DroneContext& ctx) {
    double diff = ctx.target_heading - ctx.heading;
    if (std::fabs(diff) <= TURN_STEP) {
        ctx.heading = ctx.target_heading;
        return std::make_unique<StateMoving>();
    }
    ctx.heading += (diff > 0.0 ? TURN_STEP : -TURN_STEP);
    return nullptr;
}

std::unique_ptr<IDroneState> StateDecelerating::execute(DroneContext& ctx) {
    ctx.speed -= ACCEL_STEP;
    if (ctx.speed <= 0.0) {
        ctx.speed = 0.0;
        ctx.stop_requested = false;
        return std::make_unique<StateStopped>();
    }
    return nullptr;
}
