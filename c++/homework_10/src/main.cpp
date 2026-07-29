#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include "providers/ThreadSafeTargetProvider.h"
#include "physics/DronePhysics.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/IConfigLoader.h"
#include "interfaces/ITargetProvider.h"
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

    MissionProcessor mission(std::move(solver), std::move(loader), &targets, &physics);
    if (!mission.init(argv[1])) {
        fprintf(stderr, "error: failed to init mission\n");
        return 1;
    }

    targets.start(cfg.targetTimeStep, cfg.timeScale);
    physics.start(cfg.physicsTimeStep, cfg.timeScale);
    mission.start();

    double totalTime = 10.0;
    auto end = std::chrono::steady_clock::now() +
               std::chrono::duration<double>(totalTime / cfg.timeScale);

    while (std::chrono::steady_clock::now() < end) {
        FireResult r{};
        if (mission.results().try_pop(r)) {
            printf("T=%.3f ", r.timestamp);
            if (!r.dp.ok) {
                printf("INVALID\n");
            } else if (r.dp.has_maneuver) {
                printf("FIRE %.2f %.2f MANEUVER %.2f %.2f\n",
                       r.dp.x, r.dp.y, r.dp.man_x, r.dp.man_y);
            } else {
                printf("FIRE %.2f %.2f\n", r.dp.x, r.dp.y);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    mission.stop();
    physics.stop();
    targets.stop();

    return 0;
}
