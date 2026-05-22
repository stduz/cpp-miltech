#include "factory.hpp"
#include "analytical_solver.hpp"
#include "json_target_provider.hpp"
#include "file_config_loader.hpp"

IBallisticSolver* createSolver(SolverType type) {
    switch (type) {
    case SolverType::ANALYTICAL: return new AnalyticalSolver();
    }
    return nullptr;
}

ITargetProvider* createProvider(ProviderType type, const char* param) {
    switch (type) {
    case ProviderType::JSON: {
        auto* p = new JsonTargetProvider();
        p->load(param);
        return p;
    }
    }
    return nullptr;
}

IConfigLoader* createLoader(LoaderType type) {
    switch (type) {
    case LoaderType::FILE: return new FileConfigLoader();
    }
    return nullptr;
}
