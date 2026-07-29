#pragma once
#include "drone_link.h"
#include <vector>

struct NavCmd {
    float accel;
    float turnRate;
    bool drop;
};

class Navigator {
public:
    void setAmmo(const dlink::AmmoCfg& a);
    void updateTarget(const dlink::TargetPos& t);
    NavCmd compute(const dlink::Telemetry& tel) const;
    bool ready() const;

private:
    dlink::AmmoCfg ammo_{};
    std::vector<dlink::TargetPos> targets_;
    bool has_ammo_{false};

    float fallTime(float z) const;
    float bombHDist(float speed, float t) const;
    const dlink::TargetPos* nearestTarget(float x, float y) const;
};
