#pragma once
#include "interfaces/IDroneState.h"

class StateStopped : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
};

class StateAccelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
};

class StateDecelerating : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
};

class StateTurning : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
};

class StateMoving : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneContext& ctx) override;
};
