#pragma once
#include "antidrone_turret/actuator_model.hpp"
#include <cstdint>

namespace antidrone_turret {

enum class TargetState : uint8_t { None = 0, LowConfidence = 1, Locked = 2 };
enum class TurretAction : uint8_t { Idle = 0, Track = 1 };
enum class TriggerDecision : uint8_t { Skip = 0, Requested = 1, Reloading = 2 };

struct ServoCmd {
    int8_t direction;
    float target_x;
    float error_x;
};

struct GimbalCmd {
    int8_t direction;
    float target_y;
    float error_y;
};

struct TurretDecision {
    TargetState target_state;
    TurretAction action;
    TriggerDecision trigger;
    float confidence;
    float distance_m;
};

class TurretLogic {
public:
    TurretLogic(float confidence_threshold, float max_distance_m);

    TargetState assess_target(bool visible, float confidence) const;
    ServoCmd servo_command(float x) const;
    GimbalCmd gimbal_command(float y) const;
    TriggerDecision trigger_decision(float distance_m, ActuatorState actuator_state) const;
    TurretDecision decide(bool visible, float x, float y,
                          float distance_m, float confidence,
                          ActuatorState actuator_state) const;

private:
    float confidence_threshold_;
    float max_distance_m_;
};

}  // namespace antidrone_turret
