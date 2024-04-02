#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AntiCheatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YOURPROJECT_API UAntiCheatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAntiCheatComponent();

    UFUNCTION(BlueprintCallable, Category = "AntiCheat")
    void ValidatePlayerData(FString PlayerId, float Speed, float Visibility, float Accuracy);
};
#pragma once
