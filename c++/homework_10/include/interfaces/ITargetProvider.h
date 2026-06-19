#pragma once
#include "Types.h"

class ITargetProvider {
public:
    virtual int getTargetCount() = 0;
    virtual Target getTarget(int idx) = 0;
    virtual ~ITargetProvider() = default;
};
