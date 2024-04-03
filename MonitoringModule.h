#pragma once

#include "CoreMinimal.h"
#include "MonitoringModule.generated.h"

UCLASS()
class ANTI_CHEAT_API UMonitoringModule : public UObject
{
    GENERATED_BODY()

public:
    void StartMonitoring();
    void StopMonitoring();

private:
    void MonitorMemory();
    void MonitorNetworkPackets();
    // Add other monitoring methods as needed
};
