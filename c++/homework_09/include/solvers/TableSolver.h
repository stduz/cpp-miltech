#pragma once
#include "interfaces/IBallisticSolver.h"
#include <vector>
#include <string>

class TableSolver : public IBallisticSolver {
public:
    bool loadTable(const std::string& path);
    DropPoint solve(const Vec3& drone, const Vec3& tgt,
                    double speed, double apath,
                    const AmmoParams& p) override;
private:
    std::vector<double> z0_, v0_, mass_, drag_, lift_;
    std::vector<double> data_;
    bool loaded_ = false;

    double lookup(double z0, double v0, double mass, double drag, double lift) const;
    static double clamp(double v, double lo, double hi);
    static double lerp(double a, double b, double t);
};
