#pragma once

#include <map>
#include <Windows.h>
#include <string>

struct profileStruct
{
    profileStruct() :_sum(0), _count(0){
        _startTime.QuadPart = 0;
        _min[0] = _min[1] = 0;
        _max[0] = _max[1] = 0;
    }

    void start(LARGE_INTEGER& startTime) {
        _startTime.QuadPart = startTime.QuadPart;
    }

    void update(LARGE_INTEGER& endTime) {
        if (_startTime.QuadPart == 0)
            return;

        ++_count;
        _startTime.QuadPart = 0;

        unsigned long long duration = endTime.QuadPart - _startTime.QuadPart;

        if (_min[0] > duration)
        {
            _sum += _min[0];
            _min[0] = duration;
            if (_min[1] > _min[0]){
                _min[0] = _min[1];
                _min[1] = duration;
            }
        }
        else if (duration > _max[0])
        {
            _sum += _max[0];
            _max[0] = duration;
            if (_max[0] > _max[1]) {
                _max[0] = _max[1];
                _max[1] = duration;
            }
        }
        else{
            _sum += duration;
        }

        return;
    }
    LARGE_INTEGER _startTime;

    unsigned long long int _sum;
    unsigned long long int _min[2];
    unsigned long long int _max[2];

    unsigned long long int _count;
};

class performanceProfiler
{
private:
    performanceProfiler();

    static performanceProfiler* instance;
    static SRWLOCK lock;

public:
    ~performanceProfiler();

    static performanceProfiler* getInstence();
    static void startProfile(const std::string& name);
    static void endProfile(const std::string& name);

    std::map<std::string, profileStruct*>* addThread();
    void writeProfiles();
    
private:
    static thread_local std::map<std::string, profileStruct*>* localProfileStroagePointer;
    std::map<unsigned long long int, std::map<std::string, profileStruct*>> containors;
    LARGE_INTEGER Freq;
};

