#pragma once

#include "monitor/CPUMonitoring.h"
#include "monitor/memoryMonitoring.h"

class resourceMonitor
{
public:
	resourceMonitor();
	~resourceMonitor();

	CPUTimeRate* getAllCPURate();
	CPUTimeRate* getCurrentProcessCPURate();
	std::map<std::string, CPUTimeRate>* getThreadsCPURate();
	unsigned long long int getUserMemorySize();
	unsigned long long int getUseableMemorySize();
	unsigned long long int getCurrentNonpagedMemorySize();
	unsigned long long int getSystemNonpagedMemorySize();

private:
	bool flag;
	std::thread* updateThread;
	memoryMonitoring _memory;
	CpuMonitoring* _cpu;
};