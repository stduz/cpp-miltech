#include "solvers/TableSolver.h"
#include <fstream>
#include <cmath>
#include <algorithm>

TableSolver::Interp TableSolver::findInterp(float val, const std::vector<float>& ax) {
    if (val <= ax.front()) return {0, 0.0f};
    if (val >= ax.back())  return {(int)ax.size() - 2, 1.0f};
    auto it = std::lower_bound(ax.begin(), ax.end(), val);
    int i = (int)(it - ax.begin()) - 1;
    if (i < 0) i = 0;
    float frac = (val - ax[i]) / (ax[i + 1] - ax[i]);
    return {i, frac};
}

TableSolver::Result TableSolver::lerp(const Result& a, const Result& b, float t) {
    return {a.t + (b.t - a.t) * t, a.hDist + (b.hDist - a.hDist) * t};
}

size_t TableSolver::index(int iz, int iv, int im, int id, int il) const {
    return ((((size_t)iz * v0_.size() + iv) * mass_.size() + im)
             * drag_.size() + id) * lift_.size() + il;
}

bool TableSolver::loadTable(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;

    int nz, nv, nm, nd, nl;
    f >> nz >> nv >> nm >> nd >> nl;
    if (f.fail() || nz < 2 || nv < 2 || nm < 2 || nd < 2 || nl < 2) return false;

    auto readVec = [&](std::vector<float>& v, int n) {
        v.resize(n);
        for (int i = 0; i < n; i++) f >> v[i];
    };
    readVec(z0_, nz);
    readVec(v0_, nv);
    readVec(mass_, nm);
    readVec(drag_, nd);
    readVec(lift_, nl);

    size_t total = (size_t)nz * nv * nm * nd * nl;
    data_.resize(total);
    for (size_t i = 0; i < total; i++)
        f >> data_[i].t >> data_[i].hDist;
    if (f.fail()) return false;

    loaded_ = true;
    return true;
}

TableSolver::Result TableSolver::lookup(float z0, float v0, float mass,
                                         float drag, float lift) const {
    Interp iz = findInterp(z0,   z0_);
    Interp iv = findInterp(v0,   v0_);
    Interp im = findInterp(mass, mass_);
    Interp id = findInterp(drag, drag_);
    Interp il = findInterp(lift, lift_);

    Result v[16];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      for (int c = 0; c < 2; c++)
       for (int e = 0; e < 2; e++) {
           const Result& lo = data_[index(iz.lo+a, iv.lo+b, im.lo+c, id.lo+e, il.lo)];
           const Result& hi = data_[index(iz.lo+a, iv.lo+b, im.lo+c, id.lo+e, il.lo+1)];
           v[a*8+b*4+c*2+e] = lerp(lo, hi, il.frac);
       }

    Result w[8];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      for (int c = 0; c < 2; c++)
       w[a*4+b*2+c] = lerp(v[a*8+b*4+c*2], v[a*8+b*4+c*2+1], id.frac);

    Result u[4];
    for (int a = 0; a < 2; a++)
     for (int b = 0; b < 2; b++)
      u[a*2+b] = lerp(w[a*4+b*2], w[a*4+b*2+1], im.frac);

    Result s[2];
    for (int a = 0; a < 2; a++)
        s[a] = lerp(u[a*2], u[a*2+1], iv.frac);

    return lerp(s[0], s[1], iz.frac);
}

DropPoint TableSolver::solve(const Vec3& drone, const Vec3& tgt,
                              double speed, double apath,
                              const AmmoParams& p) {
    DropPoint r{};
    if (!loaded_) return r;

    Result res = lookup((float)drone.z, (float)speed,
                        (float)p.mass, (float)p.drag, (float)p.lift);
    if (res.hDist <= 0.0f) return r;

    double dx = tgt.x - drone.x, dy = tgt.y - drone.y;
    double D = std::sqrt(dx * dx + dy * dy);
    if (D <= 0.0) return r;

    r.ok = true;
    double h = (double)res.hDist;
    if (h + apath > D) {
        r.has_maneuver = true;
        r.man_x = tgt.x - dx * (h + apath) / D;
        r.man_y = tgt.y - dy * (h + apath) / D;
    }
    double ratio = (D - h) / D;
    r.x = drone.x + dx * ratio;
    r.y = drone.y + dy * ratio;
    return r;
}
