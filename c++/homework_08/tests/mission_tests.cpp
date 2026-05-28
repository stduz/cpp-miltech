#include "solvers/AnalyticalSolver.h"
#include "config/ComponentFactory.h"
#include <gtest/gtest.h>

TEST(AnalyticalSolver, Hw1ReferencePoint) {
    AnalyticalSolver s;
    Vec3 drone{100, 100, 100};
    Vec3 tgt{200, 200, 0};
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 10.0, p);
    ASSERT_TRUE(r.ok);
    EXPECT_NEAR(r.x, 173.759, 0.5);
    EXPECT_NEAR(r.y, 173.759, 0.5);
}

TEST(AnalyticalSolver, ZeroAltitudeInvalid) {
    AnalyticalSolver s;
    Vec3 drone{0, 0, 0};
    Vec3 tgt{300, 0, 0};
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 80.0, 0.0, p);
    EXPECT_FALSE(r.ok);
}

TEST(AnalyticalSolver, ZeroDistanceInvalid) {
    AnalyticalSolver s;
    Vec3 drone{100, 100, 50};
    Vec3 tgt{100, 100, 0};
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 80.0, 0.0, p);
    EXPECT_FALSE(r.ok);
}

TEST(AnalyticalSolver, ManeuverWhenClose) {
    AnalyticalSolver s;
    Vec3 drone{0, 0, 50};
    Vec3 tgt{300, 0, 0};
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 80.0, 200.0, p);
    ASSERT_TRUE(r.ok);
    EXPECT_TRUE(r.has_maneuver);
}

TEST(ComponentFactory, CreatesSolverNotNull) {
    IBallisticSolver* s = ComponentFactory::createSolver(SolverType::ANALYTICAL);
    EXPECT_NE(s, nullptr);
    delete s;
}

TEST(ComponentFactory, CreatesLoaderNotNull) {
    IConfigLoader* l = ComponentFactory::createLoader(LoaderType::FILE);
    EXPECT_NE(l, nullptr);
    delete l;
}

TEST(FileConfigLoader, UnknownAmmoFails) {
    IConfigLoader* l = ComponentFactory::createLoader(LoaderType::FILE);
    EXPECT_FALSE(l->load("nonexistent_file.txt"));
    delete l;
}
