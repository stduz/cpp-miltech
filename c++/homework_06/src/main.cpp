#include "ballistics.hpp"

#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: ballistics <input_file>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "error: cannot open %s\n", argv[1]);
        return 1;
    }

    char ammo[32];
    BInput in{};
    in.ammo = ammo;

    while (fscanf(f, "%31s %lf %lf %lf %lf %lf %lf %lf",
                  ammo, &in.x, &in.y, &in.z,
                  &in.tx, &in.ty, &in.speed, &in.apath) == 8) {
        BResult r = compute(in);
        if (!r.ok) {
            printf("INVALID\n");
            continue;
        }
        if (r.has_maneuver)
            printf("FIRE %.2f %.2f MANEUVER %.2f %.2f\n",
                   r.fire_x, r.fire_y, r.man_x, r.man_y);
        else
            printf("FIRE %.2f %.2f\n", r.fire_x, r.fire_y);
    }

    fclose(f);
    return 0;
}
