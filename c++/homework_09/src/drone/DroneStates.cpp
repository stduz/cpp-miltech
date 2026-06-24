#include "drone/DroneStates.h"
#include <cmath>

static double normalizeAngle(double a) {
    while (a > 180.0) a -= 360.0;
    while (a < -180.0) a += 360.0;
    return a;
}

std::unique_ptr<IDroneState> StateStopped::execute(DroneContext& ctx) {
    double delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg.turnThreshold) {
        ctx.turnRemaining = std::fabs(delta) / ctx.cfg.angularSpeed;
        ctx.targetDir = ctx.desiredDir;
        return std::make_unique<StateTurning>();
    }
    ctx.direction = ctx.desiredDir;
    return std::make_unique<StateAccelerating>();
}

std::unique_ptr<IDroneState> StateAccelerating::execute(DroneContext& ctx) {
    double delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg.turnThreshold)
        return std::make_unique<StateDecelerating>();
    ctx.speed += ctx.cfg.accelStep;
    if (ctx.speed >= ctx.cfg.maxSpeed) {
        ctx.speed = ctx.cfg.maxSpeed;
        return std::make_unique<StateMoving>();
    }
    return nullptr;
}

std::unique_ptr<IDroneState> StateMoving::execute(DroneContext& ctx) {
    double delta = normalizeAngle(ctx.desiredDir - ctx.direction);
    if (std::fabs(delta) > ctx.cfg.turnThreshold)
        return std::make_unique<StateDecelerating>();
    return nullptr;
}

std::unique_ptr<IDroneState> StateTurning::execute(DroneContext& ctx) {
    double step = ctx.cfg.angularSpeed * ctx.cfg.simTimeStep;
    if (ctx.turnRemaining <= step) {
        ctx.direction = ctx.targetDir;
        ctx.turnRemaining = 0.0;
        return std::make_unique<StateAccelerating>();
    }
    double delta = normalizeAngle(ctx.targetDir - ctx.direction);
    ctx.direction += (delta > 0.0 ? step : -step);
    ctx.turnRemaining -= step;
    return nullptr;
}

std::unique_ptr<IDroneState> StateDecelerating::execute(DroneContext& ctx) {
    ctx.speed -= ctx.cfg.accelStep;
    if (ctx.speed <= 0.0) {
        ctx.speed = 0.0;
        return std::make_unique<StateStopped>();
    }
    return nullptr;
}
