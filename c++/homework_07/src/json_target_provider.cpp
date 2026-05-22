#include "json_target_provider.hpp"
#include "json.hpp"
#include <fstream>

bool JsonTargetProvider::load(const char* path) {
    std::ifstream f(path);
    if (!f) return false;
    nlohmann::json j;
    f >> j;
    for (size_t i = 0; i < j.size(); i++) {
        Target t{};
        t.pos.x = j[i]["x"].get<double>();
        t.pos.y = j[i]["y"].get<double>();
        t.pos.z = 0.0;
        targets_.push_back(t);
    }
    return !targets_.empty();
}

int JsonTargetProvider::getTargetCount() {
    return static_cast<int>(targets_.size());
}

Target JsonTargetProvider::getTarget(int idx) {
    return targets_[static_cast<size_t>(idx)];
}
