#include "solvers/TableSolver.h"
#include <fstream>
#include <cmath>
#include <algorithm>

double TableSolver::clamp(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

double TableSolver::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

bool TableSolver::loadTable(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;

    int nz, nv, nm, nd, nl;
    f >> nz >> nv >> nm >> nd >> nl;
    if (f.fail() || nz < 2 || nv < 2 || nm < 2 || nd < 2 || nl < 2) return false;

    auto readVec = [&](std::vector<double>& v, int n) {
        v.resize(n);
        for (int i = 0; i < n; i++) f >> v[i];
    };
    readVec(z0_, nz);
    readVec(v0_, nv);
    readVec(mass_, nm);
    readVec(drag_, nd);
    readVec(lift_, nl);

    int total = nz * nv * nm * nd * nl;
    data_.resize(total);
    for (int i = 0; i < total; i++) f >> data_[i];
    if (f.fail()) return false;

    loaded_ = true;
    return true;
}

double TableSolver::lookup(double z0, double v0, double mass, double drag, double lift) const {
    auto interp_idx = [](const std::vector<double>& ax, double val, int& i0, int& i1, double& t) {
        val = TableSolver::clamp(val, ax.front(), ax.back());
        i1 = static_cast<int>(std::upper_bound(ax.begin(), ax.end(), val) - ax.begin());
        if (i1 >= static_cast<int>(ax.size())) i1 = static_cast<int>(ax.size()) - 1;
        i0 = i1 - 1;
        if (i0 < 0) { i0 = 0; i1 = 1; }
        double span = ax[i1] - ax[i0];
        t = span > 0.0 ? (val - ax[i0]) / span : 0.0;
    };

    int iz0, iz1, iv0, iv1, im0, im1, id0, id1, il0, il1;
    double tz, tv, tm, td, tl;
    interp_idx(z0_, z0, iz0, iz1, tz);
    interp_idx(v0_, v0, iv0, iv1, tv);
    interp_idx(mass_, mass, im0, im1, tm);
    interp_idx(drag_, drag, id0, id1, td);
    interp_idx(lift_, lift, il0, il1, tl);

    int nv = static_cast<int>(v0_.size());
    int nm = static_cast<int>(mass_.size());
    int nd = static_cast<int>(drag_.size());
    int nl = static_cast<int>(lift_.size());

    auto idx = [&](int iz, int iv, int im, int id, int il) {
        return ((iz * nv + iv) * nm + im) * nd * nl + id * nl + il;
    };

    double c[2][2][2][2][2];
    for (int a = 0; a < 2; a++)
    for (int b = 0; b < 2; b++)
    for (int c2 = 0; c2 < 2; c2++)
    for (int d = 0; d < 2; d++)
    for (int e = 0; e < 2; e++) {
        int iz = a ? iz1 : iz0;
        int iv = b ? iv1 : iv0;
        int im = c2 ? im1 : im0;
        int id = d ? id1 : id0;
        int il = e ? il1 : il0;
        c[a][b][c2][d][e] = data_[idx(iz, iv, im, id, il)];
    }

    double r[2][2][2][2];
    for (int b = 0; b < 2; b++)
    for (int c2 = 0; c2 < 2; c2++)
    for (int d = 0; d < 2; d++)
    for (int e = 0; e < 2; e++)
        r[b][c2][d][e] = lerp(c[0][b][c2][d][e], c[1][b][c2][d][e], tz);

    double s[2][2][2];
    for (int c2 = 0; c2 < 2; c2++)
    for (int d = 0; d < 2; d++)
    for (int e = 0; e < 2; e++)
        s[c2][d][e] = lerp(r[0][c2][d][e], r[1][c2][d][e], tv);

    double u[2][2];
    for (int d = 0; d < 2; d++)
    for (int e = 0; e < 2; e++)
        u[d][e] = lerp(s[0][d][e], s[1][d][e], tm);

    double w[2];
    for (int e = 0; e < 2; e++)
        w[e] = lerp(u[0][e], u[1][e], td);

    return lerp(w[0], w[1], tl);
}

DropPoint TableSolver::solve(const Vec3& drone, const Vec3& tgt,
                              double speed, double apath,
                              const AmmoParams& p) {
    DropPoint r{};
    if (!loaded_) return r;

    double h = lookup(drone.z, speed, p.mass, p.drag, p.lift);
    if (h <= 0.0) return r;

    double dx = tgt.x - drone.x, dy = tgt.y - drone.y;
    double D = std::sqrt(dx * dx + dy * dy);
    if (D <= 0.0) return r;

    r.ok = true;
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
