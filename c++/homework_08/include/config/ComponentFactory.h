#pragma once
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "interfaces/IConfigLoader.h"
#include <string>

enum class SolverType { ANALYTICAL };
enum class ProviderType { JSON };
enum class LoaderType { FILE };

class ComponentFactory {
public:
    static IBallisticSolver* createSolver(SolverType type);
    static ITargetProvider* createProvider(ProviderType type, const std::string& param);
    static IConfigLoader* createLoader(LoaderType type);
};
