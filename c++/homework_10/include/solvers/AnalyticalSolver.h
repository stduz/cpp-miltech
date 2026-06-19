#pragma once
#include "interfaces/IBallisticSolver.h"

class AnalyticalSolver : public IBallisticSolver {
public:
    DropPoint solve(const Vec3& drone, const Vec3& tgt,
                    double speed, double apath,
                    const AmmoParams& p) override;
};
