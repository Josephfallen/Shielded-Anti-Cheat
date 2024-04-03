#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MonitoringModule.h"
#include "AnalysisModule.h"
#include "ReportingModule.h"
#include "AntiCheatManager.generated.h"

UCLASS()
class ANTI_CHEAT_API UAntiCheatManager : public UObject
{
    GENERATED_BODY()

public:
    void StartAntiCheat();
    void StopAntiCheat();

private:
    UMonitoringModule* MonitoringModule;
    UAnalysisModule* AnalysisModule;
    UReportingModule* ReportingModule;
};

