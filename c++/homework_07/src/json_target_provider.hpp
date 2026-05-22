#pragma once
#include "interfaces.hpp"
#include <vector>

class JsonTargetProvider : public ITargetProvider {
public:
    bool load(const char* path);
    int getTargetCount() override;
    Target getTarget(int idx) override;
private:
    std::vector<Target> targets_;
};
