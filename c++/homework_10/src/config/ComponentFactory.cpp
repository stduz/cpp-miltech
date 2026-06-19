#include "config/ComponentFactory.h"
#include "solvers/AnalyticalSolver.h"
#include "config/FileConfigLoader.h"

std::unique_ptr<IBallisticSolver> ComponentFactory::createSolver(SolverType type,
                                                                  const std::string&) {
    switch (type) {
    case SolverType::ANALYTICAL: return std::make_unique<AnalyticalSolver>();
    case SolverType::TABLE:      return std::make_unique<AnalyticalSolver>();
    }
    return nullptr;
}

std::unique_ptr<IConfigLoader> ComponentFactory::createLoader(LoaderType type) {
    switch (type) {
    case LoaderType::FILE: return std::make_unique<FileConfigLoader>();
    }
    return nullptr;
}
