#pragma once
#include "interfaces/ITargetProvider.h"
#include <vector>
#include <string>

class JsonTargetProvider : public ITargetProvider {
public:
    bool load(const std::string& path);
    int getTargetCount() override;
    Target getTarget(int idx) override;
private:
    std::vector<Target> targets_;
};
