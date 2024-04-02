#include "AntiCheatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "jwt-cpp/jwt.h"  // Include the jwt-cpp library header
#include "Windows/MinWindows.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

UAntiCheatComponent::UAntiCheatComponent()
{
    // Constructor
}

void UAntiCheatComponent::ValidatePlayerData(FString PlayerId, float Speed, float Visibility, float Accuracy)
{
    // Generate JWT token
    FString Token = GenerateJWTToken();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL("http://your_server_ip:5000/validate");  // Replace with your server IP address
    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("Authorization", "Bearer " + Token);  // Set JWT token in Authorization header

    FString JsonPayload = FString::Printf(TEXT("{\"player_id\": \"%s\", \"speed\": %f, \"visibility\": %f, \"accuracy\": %f}"), *PlayerId, Speed, Visibility, Accuracy);
    Request->SetContentAsString(JsonPayload);

    Request->OnProcessRequestComplete().BindUObject(this, &UAntiCheatComponent::OnValidateResponseReceived);

    Request->ProcessRequest();

    // Check for specific DLLs in memory
    if (ScanForDLLs())
    {
        // Ban the player if the target DLLs are detected
        banPlayer(PlayerId);
    }
}

void UAntiCheatComponent::OnValidateResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (bSuccess && Response.IsValid())
    {
        FString ResponseContent = Response->GetContentAsString();
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            FString Status = JsonObject->GetStringField("status");
            if (Status == "banned")
            {
                // Get the player controller
                APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
                if (PlayerController)
                {
                    // Display a VR-friendly ban message
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You have been banned for cheating!"));
                    }

                    // Delay kicking the player to allow them to read the ban message
                    FTimerHandle TimerHandle;
                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerController]()
                        {
                            // Kick the player
                            PlayerController->Kick();
                        }, 5.f, false);  // Delay kicking the player by 5 seconds
                }
            }
        }
    }
}

FString UAntiCheatComponent::GenerateJWTToken()
{
    // Create a new JWT token
    std::string secret_key = "your_secret_key";
    jwt::claims claims;
    claims.set_claim("player_id", "123456");
    claims.set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{ 3600 });

    std::string token = jwt::create()
        .set_issuer("your_issuer")
        .set_type("JWS")
        .set_payload(claims)
        .sign(jwt::algorithm::hs256{ secret_key });

    return FString(token.c_str());
}

bool UAntiCheatComponent::ScanForDLLs()
{
    // List of target DLLs to scan for
    const char* targetDLLs[] = { "nppPluginList.dll", "enco.dll" };  // Replace with your target DLL names

    // Get the current process handle
    HANDLE hProcess = GetCurrentProcess();

    // Get the list of loaded modules for the process
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szModName[MAX_PATH];
            // Get the full path to the module's file
            if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                // Check if the module name matches any of the target DLLs
                for (const char* targetDLL : targetDLLs)
                {
                    if (strstr(szModName, targetDLL))
                    {
                        // Target DLL detected
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void UAntiCheatComponent::banPlayer(FString PlayerId)
{
    // Implement ban logic here
    // For example, you can add the player's ID to a ban list and kick the player
    banList.Add(PlayerId);
    KickPlayer(PlayerId);
}

void UAntiCheatComponent::KickPlayer(FString PlayerId)
{
    // Get the player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        // Display a VR-friendly ban message
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player %s has been banned for cheating!"), *PlayerId));
        }

        // Delay kicking the player to allow them to read the ban message
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerController]()
            {
                // Kick the player
                PlayerController->Kick();
            }, 5.f, false);  // Delay kicking the player by 5 seconds
    }
}
