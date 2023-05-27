#pragma once

#include "CPUMonitoring.h"

#define monitoringCurrentThread(tag)		CPUMonitorScope t(tag)

class PDHMonitor
{

};