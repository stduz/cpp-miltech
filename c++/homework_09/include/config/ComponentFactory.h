#pragma once
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"
#include <memory>
#include <string>

enum class SolverType { ANALYTICAL, TABLE };
enum class ProviderType { JSON };
enum class LoaderType { FILE };

class ComponentFactory {
public:
    static std::unique_ptr<IBallisticSolver> createSolver(SolverType type,
                                                          const std::string& table_path = {});
    static std::unique_ptr<ITargetProvider> createProvider(ProviderType type,
                                                           const std::string& param);
    static std::unique_ptr<IConfigLoader> createLoader(LoaderType type);
};
