#include "drone/DroneController.h"
#include "drone/DroneStates.h"
#include <gtest/gtest.h>

TEST(DroneState, StartsStoppedNoTransitionWhenTargetZero) {
    DroneContext ctx{};
    ctx.target_speed = 0.0;
    StateStopped s;
    auto next = s.execute(ctx);
    EXPECT_EQ(next, nullptr);
}

TEST(DroneState, StoppedTransitionsToAcceleratingWhenTargetSet) {
    DroneContext ctx{};
    ctx.target_speed = 10.0;
    StateStopped s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
}

TEST(DroneState, AcceleratingReachesMoving) {
    DroneContext ctx{};
    ctx.speed = 9.5;
    ctx.target_speed = 10.0;
    StateAccelerating s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_NEAR(ctx.speed, 10.0, 1e-9);
}

TEST(DroneState, MovingTransitionsToDeceleratingOnStop) {
    DroneContext ctx{};
    ctx.speed = 10.0;
    ctx.target_speed = 10.0;
    ctx.heading = 0.0;
    ctx.target_heading = 0.0;
    ctx.stop_requested = true;
    StateMoving s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
}

TEST(DroneState, MovingTransitionsToTurningOnHeadingChange) {
    DroneContext ctx{};
    ctx.speed = 10.0;
    ctx.target_speed = 10.0;
    ctx.heading = 0.0;
    ctx.target_heading = 90.0;
    ctx.stop_requested = false;
    StateMoving s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
}

TEST(DroneState, DeceleratingReachesStopped) {
    DroneContext ctx{};
    ctx.speed = 0.5;
    StateDecelerating s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_NEAR(ctx.speed, 0.0, 1e-9);
}

TEST(DroneController, FullCycle) {
    DroneContext ctx{};
    ctx.speed = 0.0;
    ctx.target_speed = 2.0;
    ctx.heading = 0.0;
    ctx.target_heading = 0.0;
    ctx.stop_requested = false;

    DroneController ctrl(ctx);
    ctrl.update();
    ctrl.update();
    ctrl.update();
    EXPECT_NEAR(ctrl.context().speed, 2.0, 1e-9);
}
