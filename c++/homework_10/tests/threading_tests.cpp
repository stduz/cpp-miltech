#include "threading/ThreadSafeQueue.h"
#include "providers/ThreadSafeTargetProvider.h"
#include "physics/DronePhysics.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <cstdio>

TEST(ThreadSafeQueue, PushPop) {
    ThreadSafeQueue<int> q;
    q.push(42);
    int v = 0;
    EXPECT_TRUE(q.try_pop(v));
    EXPECT_EQ(v, 42);
}

TEST(ThreadSafeQueue, EmptyTryPop) {
    ThreadSafeQueue<int> q;
    int v = 0;
    EXPECT_FALSE(q.try_pop(v));
}

TEST(ThreadSafeQueue, MultiThreaded) {
    ThreadSafeQueue<int> q;
    std::thread producer([&]{ for (int i = 0; i < 100; i++) q.push(i); });
    producer.join();
    int sum = 0;
    int v = 0;
    while (q.try_pop(v)) sum += v;
    EXPECT_EQ(sum, 4950);
}

static const char* TARGETS_FILE = "test_targets.json";

static void write_targets() {
    std::ofstream f(TARGETS_FILE);
    f << R"([{"x":200,"y":300,"vx":1.0,"vy":0.5}])";
}

TEST(ThreadSafeTargetProvider, LoadsTargets) {
    write_targets();
    ThreadSafeTargetProvider p;
    EXPECT_TRUE(p.load(TARGETS_FILE));
    EXPECT_EQ(p.getTargetCount(), 1);
    std::remove(TARGETS_FILE);
}

TEST(ThreadSafeTargetProvider, TargetsMove) {
    write_targets();
    ThreadSafeTargetProvider p;
    p.load(TARGETS_FILE);
    Target t0 = p.getTarget(0);
    p.start(0.05, 10.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    p.stop();
    Target t1 = p.getTarget(0);
    EXPECT_GT(t1.pos.x, t0.pos.x);
    std::remove(TARGETS_FILE);
}

TEST(DronePhysics, TelemetryUpdates) {
    DronePhysics ph;
    ph.init({0,0,100}, 10.0, 0.0);
    auto t0 = ph.getTelemetry();
    ph.start(0.01, 10.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ph.stop();
    auto t1 = ph.getTelemetry();
    EXPECT_GT(t1.timestamp, t0.timestamp);
    EXPECT_GT(t1.pos.x, t0.pos.x);
}
