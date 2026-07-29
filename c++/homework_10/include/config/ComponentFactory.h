#pragma once
#include <memory>
#include <string>

class IBallisticSolver;
class IConfigLoader;

enum class SolverType { ANALYTICAL, TABLE };
enum class LoaderType { FILE };

class ComponentFactory {
public:
    static std::unique_ptr<IBallisticSolver> createSolver(SolverType type,
                                                          const std::string& table_path = {});
    static std::unique_ptr<IConfigLoader> createLoader(LoaderType type);
};
