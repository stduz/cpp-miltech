#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"

IBallisticSolver* ComponentFactory::createSolver(SolverType type) {
    switch (type) {
    case SolverType::ANALYTICAL: return new AnalyticalSolver();
    }
    return nullptr;
}

ITargetProvider* ComponentFactory::createProvider(ProviderType type, const std::string& param) {
    switch (type) {
    case ProviderType::JSON: {
        auto* p = new JsonTargetProvider();
        p->load(param);
        return p;
    }
    }
    return nullptr;
}

IConfigLoader* ComponentFactory::createLoader(LoaderType type) {
    switch (type) {
    case LoaderType::FILE: return new FileConfigLoader();
    }
    return nullptr;
}
