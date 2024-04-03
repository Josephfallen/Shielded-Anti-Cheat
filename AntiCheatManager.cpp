#include "AntiCheatManager.h"

void UAntiCheatManager::StartAntiCheat()
{
    MonitoringModule = NewObject<UMonitoringModule>();
    AnalysisModule = NewObject<UAnalysisModule>();
    ReportingModule = NewObject<UReportingModule>();

    MonitoringModule->StartMonitoring();
}

void UAntiCheatManager::StopAntiCheat()
{
    MonitoringModule->StopMonitoring();
    // Cleanup resources
}
