#pragma once

#include <map>
#include <Pdh.h>
#include <string>

class PDHMonitor;

struct CPUTimeRate
{
	CPUTimeRate() :total(0.0f), kernelMode(0.0f), userMode(0.0f){
	}
	float total;
	float kernelMode;
	float userMode;
};

struct CPUTime
{
	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;
	ULARGE_INTEGER optional;
};

class CpuMonitoring
{
	friend class PDHMonitor;

public:
	CpuMonitoring();
	~CpuMonitoring();
	void addThreadMonitor(const std::string& name);
	void removeThreadMonitor(const std::string& name);

	void update();
private:
	void updateProcessor();
	void updateProcess();
	void updateThreads();

public:
	CPUTimeRate processCpuRate;
	CPUTimeRate allCpuRate;
	std::map<std::string, CPUTimeRate> threadCpuRate;

private:
	int processorCount;
	HANDLE process;
	std::map<std::string, HANDLE> threadHandleList;

	CPUTime processorPastTime;
	CPUTime processPastTime;
	std::map<std::string, CPUTime> threadPastTime;
};