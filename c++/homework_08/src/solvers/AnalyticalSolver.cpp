#define _USE_MATH_DEFINES
#include "solvers/AnalyticalSolver.h"
#include <cmath>

static const double G = 9.81;

static double flight_time(double a, double b, double c) {
    double p = -(b * b) / (3.0 * a * a);
    double q = 2.0 * b * b * b / (27.0 * a * a * a) + c / a;
    if (p >= 0.0) return -1.0;
    double arg = (3.0 * q / (2.0 * p)) * sqrt(-3.0 / p);
    if (arg < -1.0) arg = -1.0;
    if (arg > 1.0) arg = 1.0;
    double phi = acos(arg);
    return 2.0 * sqrt(-p / 3.0) * cos((phi + 4.0 * M_PI) / 3.0) - b / (3.0 * a);
}

static double horiz_dist(double t, double v, double m, double d, double l) {
    double t2 = t * t, t3 = t2 * t, t4 = t3 * t, t5 = t4 * t;
    double l2 = l * l, l4 = l2 * l2;
    double m2 = m * m, m3 = m2 * m, m4 = m3 * m;
    double d2 = d * d, d3 = d2 * d, d4 = d3 * d;
    return v * t
        - t2 * d * v / (2.0 * m)
        + t3 * (6.0 * d * G * l * m - 6.0 * d2 * (l2 - 1.0) * v) / (36.0 * m2)
        + t4 * (-6.0 * d2 * G * l * (1.0 + l2 + l4) * m + 3.0 * d3 * l2 * (1.0 + l2) * v
                + 6.0 * d3 * l4 * (1.0 + l2) * v)
               / (36.0 * (1.0 + l2) * (1.0 + l2) * m3)
        + t5 * (3.0 * d3 * G * l * l2 * m - 3.0 * d4 * l2 * (1.0 + l2) * v)
               / (36.0 * (1.0 + l2) * m4);
}

DropPoint AnalyticalSolver::solve(const Vec3& drone, const Vec3& tgt,
                                   double speed, double apath,
                                   const AmmoParams& p) {
    DropPoint r{};
    double m = p.mass, d = p.drag, l = p.lift, v = speed, z = drone.z;
    double ac = d * G * m - 2.0 * d * d * l * v;
    double bc = -3.0 * G * m * m + 3.0 * d * l * m * v;
    double cc = 6.0 * m * m * z;
    double t = flight_time(ac, bc, cc);
    if (t <= 0.0) return r;
    double h = horiz_dist(t, v, m, d, l);
    if (h <= 0.0) return r;
    double dx = tgt.x - drone.x, dy = tgt.y - drone.y;
    double D = sqrt(dx * dx + dy * dy);
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
