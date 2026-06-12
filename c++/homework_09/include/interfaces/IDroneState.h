#pragma once
#include "Types.h"
#include <memory>

class IDroneState {
public:
    virtual std::unique_ptr<IDroneState> execute(DroneContext& ctx) = 0;
    virtual ~IDroneState() = default;
};
