#pragma once

#include "CoreMinimal.h"
#include "ReportingModule.generated.h"

UCLASS()
class ANTI_CHEAT_API UReportingModule : public UObject
{
    GENERATED_BODY()

public:
    void ReportCheating();
    // Add other reporting methods as needed
};
