#include <cmath>
#include <cstdio>

static const int    TICKS_PER_REV = 1024;
static const double WHEEL_RADIUS  = 0.3;
static const double WHEELBASE     = 1.0;
static const double DIST_PER_TICK = 2.0 * M_PI * WHEEL_RADIUS / TICKS_PER_REV;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <encoder_file>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "cannot open file: %s\n", argv[1]);
        return 1;
    }

    double x = 0.0, y = 0.0, theta = 0.0;
    long pfl = 0, pfr = 0, pbl = 0, pbr = 0;
    long ts, fl, fr, bl, br;
    bool first = true;

    while (fscanf(f, "%ld %ld %ld %ld %ld", &ts, &fl, &fr, &bl, &br) == 5) {
        if (first) {
            pfl = fl; pfr = fr; pbl = bl; pbr = br;
            first = false;
            continue;
        }

        double dL = ((fl - pfl + bl - pbl) / 2.0) * DIST_PER_TICK;
        double dR = ((fr - pfr + br - pbr) / 2.0) * DIST_PER_TICK;

        double d      = (dL + dR) / 2.0;
        double dtheta = (dR - dL) / WHEELBASE;

        x     += d * cos(theta + dtheta / 2.0);
        y     += d * sin(theta + dtheta / 2.0);
        theta += dtheta;

        printf("%ld %.4f %.4f %.4f\n", ts, x, y, theta);

        pfl = fl; pfr = fr; pbl = bl; pbr = br;
    }

    fclose(f);
    return 0;
}
