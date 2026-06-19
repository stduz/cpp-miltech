#pragma once
#include "Types.h"

class IDronePhysics {
public:
    virtual DroneTelemetry getTelemetry() const = 0;
    virtual ~IDronePhysics() = default;
};
