#include "solvers/TableSolver.h"
#include "solvers/AnalyticalSolver.h"
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <cmath>

static const char* TABLE_FILE = "test_ballistic_table.txt";

static void write_test_table() {
    // 2x2x2x2x2 table pre-computed from analytical solver values
    // axes: Z0={50,150}, V0={10,80}, mass={0.35,1.2}, drag={0.07,0.10}, lift={0.0,1.0}
    std::ofstream f(TABLE_FILE);
    f << "2 2 2 2 2\n";
    f << "50 150\n";
    f << "10 80\n";
    f << "0.35 1.2\n";
    f << "0.07 0.10\n";
    f << "0.0 1.0\n";
    // data[iz][iv][im][id][il] — horizontal distance
    double vals[32] = {
        // iz=0 (z=50)
        // iv=0 (v=10)
        // im=0 (m=0.35), id=0 (d=0.07)
        /* il=0 */ 39.5,  /* il=1 */ 41.2,
        // id=1 (d=0.10)
        /* il=0 */ 38.1,  /* il=1 */ 40.0,
        // im=1 (m=1.2)
        // id=0
        /* il=0 */ 42.3,  /* il=1 */ 40.5,
        // id=1
        /* il=0 */ 41.0,  /* il=1 */ 39.8,
        // iv=1 (v=80)
        // im=0, id=0
        /* il=0 */ 310.5, /* il=1 */ 320.1,
        // id=1
        /* il=0 */ 298.0, /* il=1 */ 308.4,
        // im=1, id=0
        /* il=0 */ 350.2, /* il=1 */ 342.0,
        // id=1
        /* il=0 */ 338.0, /* il=1 */ 330.5,
        // iz=1 (z=150)
        // iv=0
        // im=0, id=0
        /* il=0 */ 68.0,  /* il=1 */ 71.0,
        // id=1
        /* il=0 */ 65.5,  /* il=1 */ 69.0,
        // im=1, id=0
        /* il=0 */ 75.0,  /* il=1 */ 72.0,
        // id=1
        /* il=0 */ 72.0,  /* il=1 */ 70.0,
        // iv=1
        // im=0, id=0
        /* il=0 */ 540.0, /* il=1 */ 558.0,
        // id=1
        /* il=0 */ 520.0, /* il=1 */ 538.0,
        // im=1, id=0
        /* il=0 */ 610.0, /* il=1 */ 595.0,
        // id=1
        /* il=0 */ 590.0, /* il=1 */ 577.0,
    };
    for (double v : vals) f << v << " ";
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
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 0.0, p);
    EXPECT_FALSE(r.ok);
}

TEST(TableSolver, SolvesAfterLoad) {
    write_test_table();
    TableSolver s;
    s.loadTable(TABLE_FILE);
    Vec3 drone{0, 0, 50};
    Vec3 tgt{400, 0, 0};
    AmmoParams p{0.35, 0.07, 0.0};
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
    AmmoParams p{0.35, 0.07, 0.0};
    DropPoint r = s.solve(drone, tgt, 10.0, 0.0, p);
    EXPECT_TRUE(r.ok);
    std::remove(TABLE_FILE);
}

TEST(ComponentFactory, CreatesTableSolver) {
    auto s = ComponentFactory::createSolver(SolverType::TABLE);
    EXPECT_NE(s, nullptr);
}
