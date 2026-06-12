#pragma once
#include "interfaces/IDroneState.h"
#include <memory>

class DroneController {
public:
    explicit DroneController(DroneContext ctx);
    void update();
    const DroneContext& context() const;
private:
    DroneContext ctx_;
    std::unique_ptr<IDroneState> state_;
};
