#pragma once

#include <string>

#include "profiler.h"

class scopeProfiler
{
public:
    scopeProfiler(const std::string& _name);
    ~scopeProfiler();

private:
    std::string name;
};