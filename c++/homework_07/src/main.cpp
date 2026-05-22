#include "factory.hpp"
#include "mission.hpp"
#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: mission_app <config> <targets.json>\n");
        return 1;
    }

    IBallisticSolver* solver = createSolver(SolverType::ANALYTICAL);
    ITargetProvider* provider = createProvider(ProviderType::JSON, argv[2]);
    IConfigLoader* loader = createLoader(LoaderType::FILE);

    if (!provider || !solver || !loader) {
        fprintf(stderr, "error: failed to create components\n");
        delete solver;
        delete provider;
        delete loader;
        return 1;
    }

    MissionProcessor mission(provider, solver, loader);
    if (!mission.init(argv[1])) {
        fprintf(stderr, "error: failed to load config: %s\n", argv[1]);
        delete solver;
        delete provider;
        delete loader;
        return 1;
    }

    while (mission.hasNext()) {
        DropPoint dp = mission.step();
        if (!dp.ok) {
            printf("INVALID\n");
            continue;
        }
        if (dp.has_maneuver)
            printf("FIRE %.2f %.2f MANEUVER %.2f %.2f\n",
                   dp.x, dp.y, dp.man_x, dp.man_y);
        else
            printf("FIRE %.2f %.2f\n", dp.x, dp.y);
    }

    delete solver;
    delete provider;
    delete loader;
    return 0;
}
