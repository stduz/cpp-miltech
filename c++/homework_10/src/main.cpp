#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include "providers/ThreadSafeTargetProvider.h"
#include "physics/DronePhysics.h"
#include <cstdio>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: mission_app_v4 <config> <targets.json>\n");
        return 1;
    }

    auto solver = ComponentFactory::createSolver(SolverType::ANALYTICAL);
    auto loader = ComponentFactory::createLoader(LoaderType::FILE);
    if (!solver || !loader) {
        fprintf(stderr, "error: failed to create components\n");
        return 1;
    }

    if (!loader->load(argv[1])) {
        fprintf(stderr, "error: failed to load config: %s\n", argv[1]);
        return 1;
    }
    Config cfg = loader->getConfig();

    ThreadSafeTargetProvider targets;
    if (!targets.load(argv[2])) {
        fprintf(stderr, "error: failed to load targets: %s\n", argv[2]);
        return 1;
    }

    DronePhysics physics;
    physics.init(cfg.drone, cfg.speed, 0.0);

    auto loader2 = ComponentFactory::createLoader(LoaderType::FILE);
    MissionProcessor mission(std::move(solver), std::move(loader2), &targets, &physics);
    if (!mission.init(argv[1])) {
        fprintf(stderr, "error: failed to init mission\n");
        return 1;
    }

    targets.start(cfg.targetTimeStep, cfg.timeScale);
    physics.start(cfg.physicsTimeStep, cfg.timeScale);
    mission.start();

    double totalTime = 10.0;
    std::this_thread::sleep_for(
        std::chrono::duration<double>(totalTime / cfg.timeScale));

    mission.stop();
    physics.stop();
    targets.stop();

    return 0;
}
