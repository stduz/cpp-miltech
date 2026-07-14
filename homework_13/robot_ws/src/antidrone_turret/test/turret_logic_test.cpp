#include "antidrone_turret/turret_logic.hpp"
#include <gtest/gtest.h>

using namespace antidrone_turret;

static TurretLogic make_logic() { return TurretLogic(0.8f, 30.0f); }

TEST(AssessTarget, NotVisible) {
    auto l = make_logic();
    EXPECT_EQ(l.assess_target(false, 0.9f), TargetState::None);
}

TEST(AssessTarget, LowConfidence) {
    auto l = make_logic();
    EXPECT_EQ(l.assess_target(true, 0.79f), TargetState::LowConfidence);
}

TEST(AssessTarget, Locked) {
    auto l = make_logic();
    EXPECT_EQ(l.assess_target(true, 0.80f), TargetState::Locked);
    EXPECT_EQ(l.assess_target(true, 0.95f), TargetState::Locked);
}

TEST(ServoCommand, Right) {
    auto l = make_logic();
    auto cmd = l.servo_command(420.0f);
    EXPECT_EQ(cmd.direction, 1);
    EXPECT_FLOAT_EQ(cmd.target_x, 420.0f);
    EXPECT_FLOAT_EQ(cmd.error_x, 100.0f);
}

TEST(ServoCommand, Left) {
    auto l = make_logic();
    auto cmd = l.servo_command(200.0f);
    EXPECT_EQ(cmd.direction, -1);
    EXPECT_FLOAT_EQ(cmd.error_x, -120.0f);
}

TEST(ServoCommand, Center) {
    auto l = make_logic();
    auto cmd = l.servo_command(320.0f);
    EXPECT_EQ(cmd.direction, 0);
    EXPECT_FLOAT_EQ(cmd.error_x, 0.0f);
}

TEST(GimbalCommand, Up) {
    auto l = make_logic();
    auto cmd = l.gimbal_command(180.0f);
    EXPECT_EQ(cmd.direction, 1);
    EXPECT_FLOAT_EQ(cmd.target_y, 180.0f);
    EXPECT_FLOAT_EQ(cmd.error_y, 60.0f);
}

TEST(GimbalCommand, Down) {
    auto l = make_logic();
    auto cmd = l.gimbal_command(300.0f);
    EXPECT_EQ(cmd.direction, -1);
    EXPECT_FLOAT_EQ(cmd.error_y, -60.0f);
}

TEST(GimbalCommand, Center) {
    auto l = make_logic();
    auto cmd = l.gimbal_command(240.0f);
    EXPECT_EQ(cmd.direction, 0);
    EXPECT_FLOAT_EQ(cmd.error_y, 0.0f);
}

TEST(TriggerDecision, ReadyCloseTarget) {
    auto l = make_logic();
    EXPECT_EQ(l.trigger_decision(25.0f, ActuatorState::kReady), TriggerDecision::Requested);
}

TEST(TriggerDecision, ReloadingCloseTarget) {
    auto l = make_logic();
    EXPECT_EQ(l.trigger_decision(25.0f, ActuatorState::kReloading), TriggerDecision::Reloading);
}

TEST(TriggerDecision, FarTarget) {
    auto l = make_logic();
    EXPECT_EQ(l.trigger_decision(50.0f, ActuatorState::kReady), TriggerDecision::Skip);
}

TEST(TurretDecision, LockedFarTarget) {
    auto l = make_logic();
    auto dec = l.decide(true, 420.0f, 180.0f, 50.0f, 0.9f, ActuatorState::kReady);
    EXPECT_EQ(dec.target_state, TargetState::Locked);
    EXPECT_EQ(dec.action, TurretAction::Track);
    EXPECT_EQ(dec.trigger, TriggerDecision::Skip);
}

TEST(TurretDecision, LowConfidenceIdle) {
    auto l = make_logic();
    auto dec = l.decide(true, 320.0f, 240.0f, 20.0f, 0.5f, ActuatorState::kReady);
    EXPECT_EQ(dec.target_state, TargetState::LowConfidence);
    EXPECT_EQ(dec.action, TurretAction::Idle);
    EXPECT_EQ(dec.trigger, TriggerDecision::Skip);
}

TEST(TurretDecision, ReloadingPressure) {
    auto l = make_logic();
    auto dec = l.decide(true, 320.0f, 240.0f, 7.0f, 0.9f, ActuatorState::kReloading);
    EXPECT_EQ(dec.trigger, TriggerDecision::Reloading);
}
