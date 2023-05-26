#include <iostream>
#include <map>
#include <vector>

#include <Windows.h>

#include "profiler.h"

performanceProfiler performanceProfiler::instance;
thread_local std::map<std::string, profileStruct*>* performanceProfiler::localProfileStroagePointer = nullptr;
SRWLOCK performanceProfiler::lock;


void CSVstringify(const std::vector<std::string>& v, std::string& str)
{
    for (auto _str : v)
    {
        if (_str.find(',') != std::string::npos || _str.find('\"') != std::string::npos)
        {
            str += '\"';
            for (auto character : _str)
            {
                if (character == '\"')
                    str += '\"';

                str += character;
            }
            str += '\"';
            str += ",";
        }
        else
        {
            str += _str;
            str += ",";
        }
    }
    str += "\n";
}


performanceProfiler::performanceProfiler()
{
    QueryPerformanceFrequency(&Freq);
}


performanceProfiler::~performanceProfiler()
{
    writeProfiles();

    for (auto containor : containors){          //  std::map<unsigned long long int, std::map<std::string, profileStruct*>>
        for (auto item : containor.second){     //  std::map<std::string, profileStruct*>
            delete item.second;                 //  profileStruct*
        }
    }
}

performanceProfiler* performanceProfiler::getInstence()
{
    //if (instance == nullptr) {
    //    AcquireSRWLockExclusive(&lock);
    //    if (instance == nullptr) 
    //        instance = new performanceProfiler();
    //    ReleaseSRWLockExclusive(&lock);
    //}

    return &instance;
}


std::map<std::string, profileStruct*>* performanceProfiler::addThread()
{
	unsigned long long int threadKey = (GetTickCount64() << 32) | (unsigned long long int)GetCurrentThreadId();

	if (containors.find(threadKey) == containors.end()) {
		AcquireSRWLockExclusive(&lock);
		containors[threadKey] = std::map<std::string, profileStruct*>();
		ReleaseSRWLockExclusive(&lock);
	}

	return &containors[threadKey];
}


void performanceProfiler::startProfile(const std::string& name)
{
    auto instance = getInstence();
    if (localProfileStroagePointer == nullptr)
        localProfileStroagePointer = instance->addThread();
        
    auto target = localProfileStroagePointer->find(name);
    if (target == localProfileStroagePointer->end()) {
        (*localProfileStroagePointer)[name] = new profileStruct();
    }
    else if (target->second->_startTime.QuadPart != 0) {
        std::cout << "profile name overlapped : " << name << std::endl;
        abort();
    }
    LARGE_INTEGER timePoint;
    QueryPerformanceCounter(&timePoint);

    (*localProfileStroagePointer)[name]->start(timePoint);
}


void performanceProfiler::endProfile(const std::string& name)
{
    LARGE_INTEGER timePoint;
    QueryPerformanceCounter(&timePoint);

    auto instance = getInstence();
    if (localProfileStroagePointer == nullptr)
        localProfileStroagePointer = instance->addThread();

    auto target = localProfileStroagePointer->find(name);
    if (target == localProfileStroagePointer->end()) {
        (*localProfileStroagePointer)[name] = new profileStruct();
    }
    else if (target->second->_startTime.QuadPart == 0) {
        std::cout << "profile name mismatch : " << name << std::endl;
        abort();
    }

    (*localProfileStroagePointer)[name]->update(timePoint);
}

void performanceProfiler::writeProfiles()
{
    struct tm curr_tm;
    time_t t = time(0);
    localtime_s(&curr_tm, &t);

    FILE* file;
    std::string filename = ("["
        + std::to_string(curr_tm.tm_year + 1900) + "-"
        + std::to_string(curr_tm.tm_mon + 1) + "-"
        + std::to_string(curr_tm.tm_mday) + "] "
        + std::to_string(curr_tm.tm_hour) + "_"
        + std::to_string(curr_tm.tm_min) + "_"
        + std::to_string(curr_tm.tm_sec)
        + ".csv"
        );

    fopen_s(&file, filename.c_str(), "wt");

    for (auto containor : containors)
    {
        int no = 0;
        fprintf(file, "UniqueID,threadID,No,Name,Min(micro sec),Average(micro sec),Max(micro sec),Call Count\n");
        unsigned long long int uniqueID = containor.first;
        unsigned int threadID = (unsigned int)containor.first;
        for (auto item : containor.second)
        {
            std::vector<std::string> v = {
                std::to_string(uniqueID),
                std::to_string(threadID),
                std::to_string(no++) ,
                item.first ,
                std::to_string((double)item.second->_min[1] * 1000 * 1000 / Freq.QuadPart) ,
                std::to_string(((double)(item.second->_sum - item.second->_min[0] - item.second->_max[0]) * 1000 * 1000) / (item.second->_count - 2) / Freq.QuadPart),
                std::to_string((double)item.second->_max[1] * 1000 * 1000 / Freq.QuadPart),
                std::to_string(item.second->_count)
            };

            std::string str;
            CSVstringify(v, str);
            fprintf(file, str.c_str());
        }
        fprintf(file, ",,,,,,,\n");
    }

    fclose(file);
}
