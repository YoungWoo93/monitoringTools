#include <Pdh.h>
#include <tlhelp32.h>

#include "CPUMonitoring.h"

CpuMonitoring::CpuMonitoring()
{
	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);
	processorCount = SystemInfo.dwNumberOfProcessors;

	process = GetCurrentProcess();
}

CpuMonitoring::~CpuMonitoring()
{
}

void CpuMonitoring::addThreadMonitor(const std::string& name)
{
	HANDLE hCurrentThreadReal;
	
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hCurrentThreadReal, 0, FALSE, DUPLICATE_SAME_ACCESS);

	threadHandleList[name + "(" + std::to_string(GetCurrentThreadId()) + ")"] = hCurrentThreadReal;
}

void CpuMonitoring::removeThreadMonitor(const std::string& name)
{
	CloseHandle(threadHandleList[name + "(" + std::to_string(GetCurrentThreadId()) + ")"]);

	threadHandleList.erase(name + "(" + std::to_string(GetCurrentThreadId()) + ")");
	threadCpuRate.erase(name + "(" + std::to_string(GetCurrentThreadId()) + ")");
}

void CpuMonitoring::update()
{
	updateProcessor();
	updateProcess();
	updateThreads();
}

void CpuMonitoring::updateProcessor()
{
	ULARGE_INTEGER idle;
	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;

	if (GetSystemTimes((PFILETIME)&idle, (PFILETIME)&kernel, (PFILETIME)&user) == false)
		return;

	ULONGLONG kernelDiff = kernel.QuadPart - processorPastTime.kernel.QuadPart;
	ULONGLONG UserDiff = user.QuadPart - processorPastTime.user.QuadPart;
	ULONGLONG idleDiff = idle.QuadPart - processorPastTime.optional.QuadPart;

	ULONGLONG Total = kernelDiff + UserDiff;


	if (Total == 0) {
		allCpuRate.total = 0.0f;
		allCpuRate.userMode = 0.0f;
		allCpuRate.kernelMode = 0.0f;
	}
	else {
		allCpuRate.total = (float)((double)(Total - idleDiff) / Total * 100.0f);
		allCpuRate.userMode = (float)((double)UserDiff / Total * 100.0f);
		allCpuRate.kernelMode = (float)((double)(kernelDiff - idleDiff) / Total * 100.0f);
	}

	processorPastTime.kernel = kernel;
	processorPastTime.user = user;
	processorPastTime.optional = idle;
}



void CpuMonitoring::updateProcess()
{
	ULARGE_INTEGER temp;
	ULARGE_INTEGER nowTime;
	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;

	GetSystemTimeAsFileTime((LPFILETIME)&nowTime);
	GetProcessTimes(process, (LPFILETIME)&temp, (LPFILETIME)&temp, (LPFILETIME)&kernel, (LPFILETIME)&user);

	ULONGLONG kernelDiff = kernel.QuadPart - processPastTime.kernel.QuadPart;
	ULONGLONG UserDiff = user.QuadPart - processPastTime.user.QuadPart;
	ULONGLONG TimeDiff = nowTime.QuadPart - processPastTime.optional.QuadPart;

	processPastTime.kernel = kernel;
	processPastTime.user = user;
	processPastTime.optional = nowTime;


	ULONGLONG Total = kernelDiff + UserDiff;

	processCpuRate.total = (float)(Total / (double)processorCount / (double)TimeDiff * 100.0f);
	processCpuRate.kernelMode = (float)(kernelDiff / (double)processorCount / (double)TimeDiff * 100.0f);
	processCpuRate.userMode = (float)(UserDiff / (double)processorCount / (double)TimeDiff * 100.0f);
}

void CpuMonitoring::updateThreads()
{
	ULARGE_INTEGER temp;
	ULARGE_INTEGER nowTime;
	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;


	GetSystemTimeAsFileTime((LPFILETIME)&nowTime);

	for (auto it : threadHandleList)
	{
		auto ret = GetThreadTimes(it.second, (LPFILETIME)&temp, (LPFILETIME)&temp, (LPFILETIME)&kernel, (LPFILETIME)&user);

		ULONGLONG kernelDiff = kernel.QuadPart - threadPastTime[it.first].kernel.QuadPart;
		ULONGLONG UserDiff = user.QuadPart - threadPastTime[it.first].user.QuadPart;
		ULONGLONG TimeDiff = nowTime.QuadPart - threadPastTime[it.first].optional.QuadPart;

		threadPastTime[it.first].kernel = kernel;
		threadPastTime[it.first].user = user;
		threadPastTime[it.first].optional = nowTime;


		ULONGLONG Total = kernelDiff + UserDiff;

		threadCpuRate[it.first].total = (float)(Total / (double)TimeDiff * 100.0f);
		threadCpuRate[it.first].kernelMode = (float)(kernelDiff / (double)TimeDiff * 100.0f);
		threadCpuRate[it.first].userMode = (float)(UserDiff / (double)TimeDiff * 100.0f);
	}
}