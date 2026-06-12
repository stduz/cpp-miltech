#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "solvers/TableSolver.h"
#include "providers/JsonTargetProvider.h"
#include "config/FileConfigLoader.h"

std::unique_ptr<IBallisticSolver> ComponentFactory::createSolver(SolverType type,
                                                                  const std::string& table_path) {
    switch (type) {
    case SolverType::ANALYTICAL:
        return std::make_unique<AnalyticalSolver>();
    case SolverType::TABLE: {
        auto s = std::make_unique<TableSolver>();
        if (!table_path.empty()) s->loadTable(table_path);
        return s;
    }
    }
    return nullptr;
}

std::unique_ptr<ITargetProvider> ComponentFactory::createProvider(ProviderType type,
                                                                   const std::string& param) {
    switch (type) {
    case ProviderType::JSON: {
        auto p = std::make_unique<JsonTargetProvider>();
        p->load(param);
        return p;
    }
    }
    return nullptr;
}

std::unique_ptr<IConfigLoader> ComponentFactory::createLoader(LoaderType type) {
    switch (type) {
    case LoaderType::FILE:
        return std::make_unique<FileConfigLoader>();
    }
    return nullptr;
}
