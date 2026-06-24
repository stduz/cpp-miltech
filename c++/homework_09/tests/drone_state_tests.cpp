#include "drone/DroneController.h"
#include "drone/DroneStates.h"
#include <gtest/gtest.h>

static DroneContext makeCtx(double dir = 0.0, double desired = 0.0) {
    DroneContext ctx{};
    ctx.direction = dir;
    ctx.desiredDir = desired;
    ctx.cfg = {10.0, 45.0, 20.0, 2.0, 0.1};
    return ctx;
}

TEST(DroneState, StoppedNoTurnGoesToAccelerating) {
    auto ctx = makeCtx(0.0, 0.0);
    StateStopped s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Accelerating");
}

TEST(DroneState, StoppedBigTurnGoesToTurning) {
    auto ctx = makeCtx(0.0, 90.0);
    StateStopped s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Turning");
}

TEST(DroneState, AcceleratingReachesMaxSpeed) {
    auto ctx = makeCtx(0.0, 0.0);
    ctx.speed = 18.0;
    StateAccelerating s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Moving");
    EXPECT_DOUBLE_EQ(ctx.speed, 20.0);
}

TEST(DroneState, AcceleratingTurnNeededGoesToDecelerating) {
    auto ctx = makeCtx(0.0, 90.0);
    ctx.speed = 10.0;
    StateAccelerating s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Decelerating");
}

TEST(DroneState, MovingTurnNeededGoesToDecelerating) {
    auto ctx = makeCtx(0.0, 90.0);
    ctx.speed = 20.0;
    StateMoving s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Decelerating");
}

TEST(DroneState, MovingNoTurnStays) {
    auto ctx = makeCtx(0.0, 0.0);
    StateMoving s;
    auto next = s.execute(ctx);
    EXPECT_EQ(next, nullptr);
}

TEST(DroneState, DeceleratingReachesStopped) {
    auto ctx = makeCtx();
    ctx.speed = 1.0;
    StateDecelerating s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Stopped");
    EXPECT_DOUBLE_EQ(ctx.speed, 0.0);
}

TEST(DroneState, TurningFinishesGoesToAccelerating) {
    auto ctx = makeCtx(0.0, 45.0);
    ctx.targetDir = 45.0;
    ctx.turnRemaining = 1.0;
    StateTurning s;
    auto next = s.execute(ctx);
    EXPECT_NE(next, nullptr);
    EXPECT_STREQ(next->name(), "Accelerating");
}

TEST(DroneController, InitialStateIsStopped) {
    auto ctx = makeCtx(0.0, 0.0);
    DroneController ctrl(ctx);
    ctrl.update();
    EXPECT_STREQ(ctrl.stateName(), "Accelerating");
}
