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

/////////////////////////////////////////////////////////////
// example
/////////////////////////////////////////////////////////////
//
//	
//	#include <iostream>
//	#include "../monitoringTools/resourceMonitor.h"
//	
//	using namespace std;
//	
//	void testFunc1(std::string arg)
//	{
//		monitoringCurrentThread(arg);
//	
//		int i = 0;
//		for (int i = 0; i < 1000000000; i++)
//			Sleep(100);
//	}
//	
//	void testFunc2(std::string arg)
//	{
//		monitoringCurrentThread(arg);
//	
//		for (int i = 0; i < 10000000; i--)
//			i++;
//	}
//	
//	void main()
//	{
//		resourceMonitor testPDH;
//		thread t1(testFunc1, "sleep");
//		thread t2(testFunc2, "add");
//	
//		while (true)
//		{
//			Sleep(100);
//	
//			cout << testPDH.getAllCPURate()->total << endl;
//			cout << testPDH.getUserMemorySize() << endl << endl;
//			for (auto it : *testPDH.getThreadsCPURate()) {
//				cout << "\t" << it.first << "\t:\t" << it.second.total << endl;
//			}
//		}
//	}
//
/////////////////////////////////////////////////////////////
