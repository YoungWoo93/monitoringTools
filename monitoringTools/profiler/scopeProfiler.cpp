#include "profiler.h"
#include "scopeProfiler.h"

scopeProfiler::scopeProfiler(const std::string& _name) : name(_name)
{
    performanceProfiler::startProfile(name);
}

scopeProfiler::~scopeProfiler()
{
    performanceProfiler::endProfile(name);
}