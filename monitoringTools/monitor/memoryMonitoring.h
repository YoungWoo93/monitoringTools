#pragma once
#include <Pdh.h>

class resourceMonitor;

class memoryMonitoring
{
	friend class resourceMonitor;
private:
	memoryMonitoring();

public:
	~memoryMonitoring();
	void update();

public:
	unsigned long long int userMemory;
	unsigned long long int useableMemory;
	unsigned long long int processNonpagedMemory;
	unsigned long long int systemNonpagedMemory;

private:
	PDH_HQUERY query;

	PDH_HCOUNTER  userMemoryCounter;
	PDH_HCOUNTER  useableMemoryCounter;
	PDH_HCOUNTER  processNonpagedMemoryCounter;
	PDH_HCOUNTER  systemNonpagedMemoryCounter;
};