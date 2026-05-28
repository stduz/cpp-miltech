#include "providers/JsonTargetProvider.h"
#include "json.hpp"
#include <fstream>

bool JsonTargetProvider::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    nlohmann::json j;
    f >> j;
    for (const auto& item : j.arr_) {
        Target t{};
        t.pos.x = item["x"].get<double>();
        t.pos.y = item["y"].get<double>();
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
