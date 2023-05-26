#pragma once

#include "profiler/profiler.h"
#include "profiler/scopeProfiler.h"
															
/////////////////////////////////////////////////////////////
// example
/////////////////////////////////////////////////////////////
//
//	#include <thread>
//	#include "monitoringTools/performanceProfiler.h"
//	
//	void testFunc1()
//	{
//		for (int i = 0; i < 100; i++)
//		{
//			performanceProfiler::startProfile("test1");
//			for (int j = 0; j < 10000; j++) {
//				j++;
//			}
//			performanceProfiler::endProfile("test1");
//		}
//	}
//	
//	void testFunc2()
//	{
//		for (int i = 0; i < 100; i++)
//		{
//			scopeProfiler a("test2");
//			for (int j = 0; j < 10000; j++) {
//				j++;
//			}
//		}
//	}
// 
//	void main()
//	{
//		std::thread t1(testFunc1);
//		std::thread t2(testFunc2);
//	
//		t1.join();
//		t2.join();
//	}
//
/////////////////////////////////////////////////////////////