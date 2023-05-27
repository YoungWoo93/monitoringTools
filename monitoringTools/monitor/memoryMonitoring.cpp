#include <Pdh.h>
#include <string>

#include "memoryMonitoring.h"

#pragma comment(lib,"Pdh.lib")

memoryMonitoring::memoryMonitoring()
{
	PdhOpenQuery(NULL, NULL, &query);

	char buffer[MAX_PATH];
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	std::string temp;
	std::string processName = std::string(strrchr(buffer, '\\') + 1, strrchr(buffer, '.'));

	temp = "\\Process(" + processName + ")\\Private Bytes";
	PdhAddCounterA(query, temp.c_str(), NULL, &userMemoryCounter);
	temp = "\\Process(" + processName + ")\\Pool Nonpaged Bytes";
	PdhAddCounterA(query, temp.c_str(), NULL, &processNonpagedMemoryCounter);
	PdhAddCounterA(query, "\\Memory\\Available MBytes", NULL, &useableMemoryCounter);
	PdhAddCounterA(query, "\\Memory\\Pool Nonpaged Bytes", NULL, &systemNonpagedMemoryCounter);

	PdhCollectQueryData(query);
}


memoryMonitoring::~memoryMonitoring(){
}

void memoryMonitoring::update()
{
	PdhCollectQueryData(query);

	PDH_FMT_COUNTERVALUE temp;
	PdhGetFormattedCounterValue(userMemoryCounter, PDH_FMT_LARGE, NULL, &temp);
	userMemory = temp.largeValue;
	PdhGetFormattedCounterValue(useableMemoryCounter, PDH_FMT_LARGE, NULL, &temp);
	useableMemory = temp.largeValue * 1024 * 1024;
	PdhGetFormattedCounterValue(processNonpagedMemoryCounter, PDH_FMT_LARGE, NULL, &temp);
	processNonpagedMemory = temp.largeValue;
	PdhGetFormattedCounterValue(systemNonpagedMemoryCounter, PDH_FMT_LARGE, NULL, &temp);
	systemNonpagedMemory = temp.largeValue;
}