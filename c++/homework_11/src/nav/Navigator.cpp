#include "nav/Navigator.h"
#include <cmath>
#include <algorithm>

static constexpr float G = 9.81f;
static constexpr float PI_F = 3.14159265358979f;

void Navigator::setAmmo(const dlink::AmmoCfg& a) {
    ammo_ = a;
    has_ammo_ = true;
    targets_.reserve(a.nTargets);
}

void Navigator::updateTarget(const dlink::TargetPos& t) {
    for (auto& existing : targets_) {
        if (existing.id == t.id) { existing = t; return; }
    }
    targets_.push_back(t);
}

bool Navigator::ready() const {
    return has_ammo_ && !targets_.empty();
}

float Navigator::fallTime(float z) const {
    return (z > 0.0f) ? sqrtf(2.0f * z / G) : 0.0f;
}

float Navigator::bombHDist(float speed, float t) const {
    if (ammo_.drag > 1e-6f) {
        float k = ammo_.drag / ammo_.mass;
        return speed / k * (1.0f - expf(-k * t));
    }
    return speed * t;
}

const dlink::TargetPos* Navigator::nearestTarget(float x, float y) const {
    if (targets_.empty()) return nullptr;
    const dlink::TargetPos* best = &targets_[0];
    float best_d = (best->x - x)*(best->x - x) + (best->y - y)*(best->y - y);
    for (size_t i = 1; i < targets_.size(); i++) {
        float d = (targets_[i].x - x)*(targets_[i].x - x)
                + (targets_[i].y - y)*(targets_[i].y - y);
        if (d < best_d) { best_d = d; best = &targets_[i]; }
    }
    return best;
}

NavCmd Navigator::compute(const dlink::Telemetry& tel) const {
    if (!has_ammo_ || targets_.empty()) return {1.0f, 0.0f, false};

    const dlink::TargetPos* tgt = nearestTarget(tel.x, tel.y);

    float t = fallTime(tel.z);
    float hd = bombHDist(tel.speed, t);

    float land_x = tel.x + hd * cosf(tel.dir);
    float land_y = tel.y + hd * sinf(tel.dir);
    float ex = land_x - tgt->x;
    float ey = land_y - tgt->y;
    bool drop = (ex*ex + ey*ey) <= ammo_.hitRadius * ammo_.hitRadius;

    float dx = tgt->x - tel.x;
    float dy = tgt->y - tel.y;
    float dist = sqrtf(dx*dx + dy*dy);
    float inv_dist = (dist > 1e-3f) ? 1.0f / dist : 0.0f;
    float nx = dx * inv_dist;
    float ny = dy * inv_dist;

    float lead_x = tgt->x - hd * nx;
    float lead_y = tgt->y - hd * ny;
    float ldx = lead_x - tel.x;
    float ldy = lead_y - tel.y;

    float desired = (ldx*ldx + ldy*ldy > 1.0f) ? atan2f(ldy, ldx) : atan2f(dy, dx);
    float err = desired - tel.dir;
    while (err >  PI_F) err -= 2*PI_F;
    while (err < -PI_F) err += 2*PI_F;

    float turnRate = std::clamp(err / (PI_F / 4.0f), -1.0f, 1.0f);
    return {1.0f, turnRate, drop};
}
