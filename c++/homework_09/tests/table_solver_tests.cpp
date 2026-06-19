#include "solvers/TableSolver.h"
#include "config/ComponentFactory.h"
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>

static const char* TABLE_FILE = "test_ballistic_table.txt";

static void write_test_table() {
    std::ofstream f(TABLE_FILE);
    // 2x2x2x2x2, axes: Z0={50,150}, V0={10,80}, mass={0.35,1.2}, drag={0.004,0.007}, lift={0,0.005}
    f << "2 2 2 2 2\n";
    f << "50 150\n";
    f << "10 80\n";
    f << "0.35 1.2\n";
    f << "0.004 0.007\n";
    f << "0.0 0.005\n";
    // 32 entries: t hDist per node
    float vals[32][2] = {
        {4.2f,39.5f},{4.3f,41.2f},{4.1f,38.1f},{4.2f,40.0f},
        {4.4f,42.3f},{4.2f,40.5f},{4.3f,41.0f},{4.1f,39.8f},
        {3.8f,310.5f},{3.9f,320.1f},{3.7f,298.0f},{3.8f,308.4f},
        {4.1f,350.2f},{3.9f,342.0f},{4.0f,338.0f},{3.8f,330.5f},
        {7.1f,68.0f},{7.3f,71.0f},{6.9f,65.5f},{7.1f,69.0f},
        {7.5f,75.0f},{7.2f,72.0f},{7.3f,72.0f},{7.0f,70.0f},
        {6.5f,540.0f},{6.7f,558.0f},{6.3f,520.0f},{6.5f,538.0f},
        {7.0f,610.0f},{6.8f,595.0f},{6.9f,590.0f},{6.7f,577.0f},
    };
    for (auto& v : vals) f << v[0] << " " << v[1] << " ";
    f << "\n";
}

TEST(TableSolver, LoadsTable) {
    write_test_table();
    TableSolver s;
    EXPECT_TRUE(s.loadTable(TABLE_FILE));
    std::remove(TABLE_FILE);
}

TEST(TableSolver, InvalidFileReturnsFalse) {
    TableSolver s;
    EXPECT_FALSE(s.loadTable("nonexistent_table.txt"));
}

TEST(TableSolver, UnloadedReturnsInvalid) {
    TableSolver s;
    Vec3 drone{100, 100, 100};
    Vec3 tgt{200, 200, 0};
    AmmoParams p{0.35, 0.004, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 0.0, p);
    EXPECT_FALSE(r.ok);
}

TEST(TableSolver, SolvesAfterLoad) {
    write_test_table();
    TableSolver s;
    s.loadTable(TABLE_FILE);
    Vec3 drone{0, 0, 50};
    Vec3 tgt{400, 0, 0};
    AmmoParams p{0.35, 0.004, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 0.0, p);
    EXPECT_TRUE(r.ok);
    std::remove(TABLE_FILE);
}

TEST(TableSolver, ClampsOutOfBounds) {
    write_test_table();
    TableSolver s;
    s.loadTable(TABLE_FILE);
    Vec3 drone{0, 0, 200};
    Vec3 tgt{400, 0, 0};
    AmmoParams p{0.35, 0.004, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 0.0, p);
    EXPECT_TRUE(r.ok);
    std::remove(TABLE_FILE);
}

TEST(ComponentFactory, CreatesTableSolver) {
    auto s = ComponentFactory::createSolver(SolverType::TABLE);
    EXPECT_NE(s, nullptr);
}
