#include "ballistics.hpp"

#include <gtest/gtest.h>
#include <cmath>

static bool near(double a, double b, double eps = 0.5) {
    return std::fabs(a - b) < eps;
}

TEST(Ballistics, UnknownAmmoReturnsInvalid) {
    BInput in{0, 0, 50, 300, 400, 80, 0, "UNKNOWN"};
    BResult r = compute(in);
    EXPECT_FALSE(r.ok);
}

TEST(Ballistics, ZeroDistanceReturnsInvalid) {
    BInput in{100, 100, 50, 100, 100, 80, 0, "VOG-17"};
    BResult r = compute(in);
    EXPECT_FALSE(r.ok);
}

TEST(Ballistics, Vog17BasicOk) {
    BInput in{0, 0, 50, 300, 400, 80, 0, "VOG-17"};
    BResult r = compute(in);
    EXPECT_TRUE(r.ok);
    EXPECT_FALSE(r.has_maneuver);
}

TEST(Ballistics, Vog17FirePointOnLine) {
    BInput in{0, 0, 50, 300, 0, 80, 0, "VOG-17"};
    BResult r = compute(in);
    ASSERT_TRUE(r.ok);
    EXPECT_NEAR(r.fire_y, 0.0, 0.01);
    EXPECT_GT(r.fire_x, 0.0);
    EXPECT_LT(r.fire_x, 300.0);
}

TEST(Ballistics, ManeuverPointWhenApathLarge) {
    BInput in{0, 0, 50, 300, 0, 80, 200, "VOG-17"};
    BResult r = compute(in);
    ASSERT_TRUE(r.ok);
    EXPECT_TRUE(r.has_maneuver);
    EXPECT_NEAR(r.man_y, 0.0, 0.01);
}

TEST(Ballistics, GlidingVogOk) {
    BInput in{0, 0, 60, 400, 0, 90, 0, "GLIDING-VOG"};
    BResult r = compute(in);
    EXPECT_TRUE(r.ok);
}

TEST(Ballistics, M67Ok) {
    BInput in{0, 0, 30, 200, 0, 60, 0, "M67"};
    BResult r = compute(in);
    EXPECT_TRUE(r.ok);
}

TEST(Ballistics, AllAmmoTypes) {
    const char* types[] = {"VOG-17", "M67", "RKG-3", "GLIDING-VOG", "GLIDING-RKG"};
    for (const char* t : types) {
        BInput in{0, 0, 50, 300, 0, 80, 0, t};
        BResult r = compute(in);
        EXPECT_TRUE(r.ok) << "failed for " << t;
    }
}
