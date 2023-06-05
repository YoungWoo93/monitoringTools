#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <DbgHelp.h>
#include <minidumpapiset.h>

#pragma comment(lib, "Dbghelp.lib")

class dump
{
public:
	dump()
	{
		_invalid_parameter_handler oldHandler, newHandler;
		newHandler = myInvalidParameterHandler;

		oldHandler = _set_invalid_parameter_handler(newHandler); // crt 함수에 null 포인터 등을 넣었을 때..
		// CRT 오류 메시지 표시 중단. 바로 덤프로 남도록.
		// CRT 오류 메시지 표시 중단, 바로 덤프로 남도록.
		// CRT 오류 메시지 표시 중단. 바로 덤프로 남도록
		_CrtSetReportMode(_CRT_WARN, 0);
		_CrtSetReportMode(_CRT_ASSERT, 0);
		_CrtSetReportMode(_CRT_ERROR, 0);
		_CrtSetReportHook(_custom_Report_hook);

		_set_purecall_handler(myPurecallHandler);
		SetHandlerDump();
	}


	static void crash(void)
	{
		int* p = nullptr;
		*p = 0;
	}
	static LONG WINAPI exceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
	{
		int iWorkingMemory = 0;
		SYSTEMTIME stNowTime;

		long DumpCount = InterlockedIncrement((LONG*)&dumpCount);

		HANDLE hProcess = 0;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = GetCurrentProcess();

		if (NULL == hProcess)
			return 0;

		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
		}

		CloseHandle(hProcess);

		char filename[MAX_PATH];
		GetLocalTime(&stNowTime);
		sprintf_s(filename, MAX_PATH,"Dump_%d %02d %02d %02d.%02d.%02d_%d %dMB.dmp",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkingMemory);
		printf("\n\n\n!!!Crash Error!!!%d.%d.%d / %d:%d:%d\n",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
		printf("Now Save dump file... \n");

		HANDLE file = ::CreateFileA(filename,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (file != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptioninformation;

			MinidumpExceptioninformation.ThreadId = ::GetCurrentThreadId();
			MinidumpExceptioninformation.ExceptionPointers = pExceptionPointer;
			MinidumpExceptioninformation.ClientPointers = TRUE;

			::MiniDumpWriteDump(GetCurrentProcess(),
				GetCurrentProcessId(),
				file,
				MiniDumpWithFullMemory,
				&MinidumpExceptioninformation,
				NULL,
				NULL);

			CloseHandle(file);
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void SetHandlerDump()
	{
		SetUnhandledExceptionFilter(exceptionFilter);
	}

	static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
	{
		crash();
	}

	static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue)
	{
		crash();
		return true;
	}

	static void myPurecallHandler(void)
	{
		crash();
	}

	static int dumpCount;
};