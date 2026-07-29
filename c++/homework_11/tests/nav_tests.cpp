#include "nav/Navigator.h"
#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

using namespace dlink;

static AmmoCfg make_ammo(float hit = 15.0f) {
    AmmoCfg a{};
    std::strncpy(a.name, "VOG-17", sizeof a.name);
    a.mass = 0.35f;
    a.drag = 0.004f;
    a.lift = 0.0f;
    a.hitRadius = hit;
    a.nTargets = 1;
    return a;
}

static Telemetry make_tel(float x, float y, float z, float speed, float dir) {
    Telemetry t{};
    t.x = x; t.y = y; t.z = z;
    t.speed = speed; t.dir = dir;
    t.vx = speed * cosf(dir);
    t.vy = speed * sinf(dir);
    return t;
}

TEST(Navigator, NotReadyWithoutAmmo) {
    Navigator nav;
    TargetPos tp{0, 300.0f, 200.0f};
    nav.updateTarget(tp);
    auto cmd = nav.compute(make_tel(0, 0, 150, 20, 0));
    EXPECT_FALSE(cmd.drop);
    EXPECT_FALSE(nav.ready());
}

TEST(Navigator, NotReadyWithoutTarget) {
    Navigator nav;
    nav.setAmmo(make_ammo());
    EXPECT_FALSE(nav.ready());
}

TEST(Navigator, ReadyWhenBothSet) {
    Navigator nav;
    nav.setAmmo(make_ammo());
    nav.updateTarget({0, 100.0f, 0.0f});
    EXPECT_TRUE(nav.ready());
}

TEST(Navigator, DropWhenAligned) {
    Navigator nav;
    nav.setAmmo(make_ammo(20.0f));

    float z = 150.0f;
    float speed = 20.0f;
    float t = sqrtf(2.0f * z / 9.81f);
    float k = 0.004f / 0.35f;
    float hd = speed / k * (1.0f - expf(-k * t));

    // Drone at origin, flying east (dir=0), target exactly at bomb landing point
    nav.updateTarget({0, hd, 0.0f});
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, z, speed, 0.0f));
    EXPECT_TRUE(cmd.drop);
}

TEST(Navigator, NoDropWhenNotAligned) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    // Target far to the north, drone flying east — no chance of hitting yet
    nav.updateTarget({0, 0.0f, 2000.0f});
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 150.0f, 20.0f, 0.0f));
    EXPECT_FALSE(cmd.drop);
}

TEST(Navigator, TurnLeftTowardNorthTarget) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, 0.0f, 500.0f});
    // Drone flying east (dir=0), target is north → must turn left (+turnRate)
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 15.0f, 0.0f));
    EXPECT_GT(cmd.turnRate, 0.0f);
}

TEST(Navigator, TurnRightTowardSouthTarget) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, 0.0f, -500.0f});
    // Drone flying east, target south → must turn right (-turnRate)
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 15.0f, 0.0f));
    EXPECT_LT(cmd.turnRate, 0.0f);
}

TEST(Navigator, TurnRateClamped) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, -500.0f, 0.0f});
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 15.0f, 0.0f));
    EXPECT_LE(std::abs(cmd.turnRate), 1.0f);
}

TEST(Navigator, AlwaysFullAccel) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, 400.0f, 0.0f});
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 20.0f, 0.0f));
    EXPECT_FLOAT_EQ(cmd.accel, 1.0f);
}

TEST(Navigator, MultiTargetPicksNearest) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, 100.0f,  0.0f});  // north, near
    nav.updateTarget({1, 0.0f, -2000.0f}); // south, far
    // Drone at origin flying east — nearest is id=0 (north), so should turn left
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 15.0f, 0.0f));
    EXPECT_GT(cmd.turnRate, 0.0f);
}

TEST(Navigator, UpdateExistingTarget) {
    Navigator nav;
    nav.setAmmo(make_ammo(15.0f));
    nav.updateTarget({0, 100.0f, 0.0f});
    nav.updateTarget({0, 200.0f, 0.0f}); // update same id
    EXPECT_TRUE(nav.ready());
    // Should still have only 1 target (not 2 duplicates)
    // Navigate: target is now at (200,0), drone at origin flying east → turnRate ~0
    auto cmd = nav.compute(make_tel(0.0f, 0.0f, 100.0f, 15.0f, 0.0f));
    EXPECT_LE(std::abs(cmd.turnRate), 1.0f);
}
