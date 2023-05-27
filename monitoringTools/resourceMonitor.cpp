#include <thread>

#include "resourceMonitor.h"

resourceMonitor::resourceMonitor() {
	_cpu = CpuMonitoring::getInstance();
	flag = true;

	updateThread = new std::thread([this]{
		while (flag)
		{
			Sleep(1000);
			_cpu->update();
			_memory.update();
		}
		});
};
resourceMonitor::~resourceMonitor() 
{
	flag = false;
	updateThread->join();
}

CPUTimeRate* resourceMonitor::getAllCPURate()
{
	return &_cpu->allCpuRate;
}
CPUTimeRate* resourceMonitor::getCurrentProcessCPURate()
{
	return &_cpu->processCpuRate;
}
std::map<std::string, CPUTimeRate>* resourceMonitor::getThreadsCPURate()
{
	return &_cpu->threadCpuRate;
}
unsigned long long int resourceMonitor::getUserMemorySize()
{
	return _memory.userMemory;
}
unsigned long long int resourceMonitor::getUseableMemorySize()
{
	return _memory.useableMemory;
}
unsigned long long int resourceMonitor::getCurrentNonpagedMemorySize()
{
	return _memory.processNonpagedMemory;
}
unsigned long long int resourceMonitor::getSystemNonpagedMemorySize()
{
	return _memory.systemNonpagedMemory;
}
