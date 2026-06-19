#pragma once
#include "Types.h"

class IBallisticSolver {
public:
    virtual DropPoint solve(const Vec3& drone, const Vec3& tgt,
                            double speed, double apath,
                            const AmmoParams& p) = 0;
    virtual ~IBallisticSolver() = default;
};
