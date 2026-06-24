#pragma once
#include "interfaces/IBallisticSolver.h"
#include <vector>
#include <string>

class TableSolver : public IBallisticSolver {
public:
    struct Result { float t; float hDist; };

    bool loadTable(const std::string& path);
    DropPoint solve(const Vec3& drone, const Vec3& tgt,
                    double speed, double apath,
                    const AmmoParams& p) override;
private:
    std::vector<float> z0_, v0_, mass_, drag_, lift_;
    std::vector<Result> data_;
    bool loaded_ = false;

    struct Interp { int lo; float frac; };
    static Interp findInterp(float val, const std::vector<float>& ax);
    static Result lerp(const Result& a, const Result& b, float t);
    Result lookup(float z0, float v0, float mass, float drag, float lift) const;
    size_t index(int iz, int iv, int im, int id, int il) const;
};
