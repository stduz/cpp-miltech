#include "MissionProcessor.h"
#include "config/ComponentFactory.h"
#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: mission_app_v3 <config> <targets.json>\n");
        return 1;
    }

    auto solver = ComponentFactory::createSolver(SolverType::ANALYTICAL);
    auto provider = ComponentFactory::createProvider(ProviderType::JSON, argv[2]);
    auto loader = ComponentFactory::createLoader(LoaderType::FILE);

    MissionProcessor mission(std::move(provider), std::move(solver), std::move(loader));
    if (!mission.init(argv[1])) {
        fprintf(stderr, "error: failed to load config: %s\n", argv[1]);
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

    return 0;
}
