#include "antidrone_turret/turret_logic.hpp"

namespace antidrone_turret {

TurretLogic::TurretLogic(float confidence_threshold, float max_distance_m)
    : confidence_threshold_(confidence_threshold), max_distance_m_(max_distance_m) {}

TargetState TurretLogic::assess_target(bool visible, float confidence) const {
    if (!visible) return TargetState::None;
    if (confidence < confidence_threshold_) return TargetState::LowConfidence;
    return TargetState::Locked;
}

ServoCmd TurretLogic::servo_command(float x) const {
    float err = x - 320.0f;
    int8_t dir = (err > 0.0f) ? 1 : (err < 0.0f) ? -1 : 0;
    return {dir, x, err};
}

GimbalCmd TurretLogic::gimbal_command(float y) const {
    float err = 240.0f - y;
    int8_t dir = (err > 0.0f) ? 1 : (err < 0.0f) ? -1 : 0;
    return {dir, y, err};
}

TriggerDecision TurretLogic::trigger_decision(float distance_m, ActuatorState actuator_state) const {
    if (distance_m > max_distance_m_) return TriggerDecision::Skip;
    if (actuator_state == ActuatorState::kReady) return TriggerDecision::Requested;
    return TriggerDecision::Reloading;
}

TurretDecision TurretLogic::decide(bool visible, float x, float y,
                                    float distance_m, float confidence,
                                    ActuatorState actuator_state) const {
    auto ts = assess_target(visible, confidence);
    if (ts != TargetState::Locked) {
        return {ts, TurretAction::Idle, TriggerDecision::Skip, confidence, distance_m};
    }
    auto trig = trigger_decision(distance_m, actuator_state);
    (void)x; (void)y;
    return {ts, TurretAction::Track, trig, confidence, distance_m};
}

}  // namespace antidrone_turret
