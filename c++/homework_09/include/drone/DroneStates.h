#pragma once
#include "interfaces/IDroneState.h"

class StateStopped : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "Stopped"; }
};

class StateAccelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "Accelerating"; }
};

class StateDecelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "Decelerating"; }
};

class StateTurning : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "Turning"; }
};

class StateMoving : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
    const char* name() const override { return "Moving"; }
};
