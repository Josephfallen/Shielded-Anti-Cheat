#include "AntiCheatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Http.h"
#include "Json.h"
#include "jwt-cpp/jwt.h"
#include "Windows/MinWindows.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

UAntiCheatComponent::UAntiCheatComponent()
{
    // Constructor
}

UAntiCheatComponent::~UAntiCheatComponent()
{
    // Destructor
}

bool UAntiCheatComponent::IsDebuggerPresent() const
{
    return ::IsDebuggerPresent();
}

bool UAntiCheatComponent::IsInsideVirtualMachine() const
{
    int32 CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 1);

    bool isVM = false;

    // Check for hypervisor bit (bit 31 of ECX register)
    if (CPUInfo[2] & (1 << 31))
    {
        isVM = true;
    }

    return isVM;
}

bool UAntiCheatComponent::ScanForHooks() const
{
    const char* hookLibs[] = { "Detours.dll", "MSHook.dll" };

    HANDLE hProcess = GetCurrentProcess();
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                for (const char* hookLib : hookLibs)
                {
                    if (strstr(szModName, hookLib))
                    {
                        return true;
                    }
                }
            }
        }
    }

    // TODO: Check for suspicious API hooks

    return false;
}

void UAntiCheatComponent::ValidateGameTime()
{
    float CurrentGameTime = GetWorld()->GetTimeSeconds();
    float ServerTime = 0.0f;  // Replace with actual server time

    if (FMath::Abs(CurrentGameTime - ServerTime) > 1.0f)
    {
        // Assuming PlayerId is a class member variable
        KickPlayer(PlayerId);
    }
}

void UAntiCheatComponent::ValidatePlayerData(const FString& PlayerId, float Speed, float Visibility, float Accuracy)
{
    // Generate JWT token
    FString Token = GenerateJWTToken();

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL("http://127.0.0.1:5000/validate");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("Authorization", "Bearer " + Token);

    FString JsonPayload = FString::Printf(TEXT("{\"player_id\": \"%s\", \"speed\": %f, \"visibility\": %f, \"accuracy\": %f}"), *PlayerId, Speed, Visibility, Accuracy);
    Request->SetContentAsString(JsonPayload);

    Request->OnProcessRequestComplete().BindUObject(this, &UAntiCheatComponent::OnValidateResponseReceived);

    Request->ProcessRequest();

    if (IsDebuggerPresent() || IsInsideVirtualMachine() || ScanForHooks())
    {
        KickPlayer(PlayerId);
        return;
    }

    ValidateGameTime();

    if (ScanForDLLs())
    {
        BanPlayer(PlayerId);
        return;
    }

    // Assuming GameFilePath, ExpectedChecksum, MemoryAddress, and MemorySize are class member variables
    bool IsFileIntegrityValid = CheckFileIntegrity(GameFilePath, ExpectedChecksum);
    bool IsMemoryProtected = ProtectMemory(MemoryAddress, MemorySize);

    if (!IsFileIntegrityValid || !IsMemoryProtected)
    {
        KickPlayer(PlayerId);
        return;
    }

    FVector ServerPosition = FVector::ZeroVector;  // Replace with actual server position
    FVector ClientPosition = FVector::ZeroVector;  // Replace with actual client position
    int32 ServerScore = 0;  // Replace with actual server score
    int32 ClientScore = 0;  // Replace with actual client score

    bool IsValidState = ValidatePlayerPosition(ServerPosition, ClientPosition, 10.0f);
    bool IsValidScore = ValidatePlayerScore(ServerScore, ClientScore);

    if (!IsValidState || !IsValidScore)
    {
        KickPlayer(PlayerId);
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
                APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
                if (PlayerController)
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You have been banned for cheating!"));
                    }
                    FTimerHandle TimerHandle;
                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerController]()
                    {
                        PlayerController->Kick();
                    }, 5.f, false);
                }
            }
        }
    }
}

FString UAntiCheatComponent::GenerateJWTToken() const
{
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

bool UAntiCheatComponent::ScanForDLLs() const
{
    const char* targetDLLs[] = { "nppPluginList.dll", "enco.dll" };

    HANDLE hProcess = GetCurrentProcess();
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szModName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                for (const char* targetDLL : targetDLLs)
                {
                    if (strstr(szModName, targetDLL))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void UAntiCheatComponent::BanPlayer(const FString& PlayerId)
{
    BanList.Add(PlayerId);
    KickPlayer(PlayerId);
}

void UAntiCheatComponent::UnbanPlayer(const FString& PlayerId)
{
    BanList.Remove(PlayerId);
}

void UAntiCheatComponent::KickPlayer(const FString& PlayerId)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player %s has been kicked for cheating!"), *PlayerId));
        }
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerController]()
        {
            PlayerController->Kick();
        }, 5.f, false);
    }
}

bool UAntiCheatComponent::CheckFileIntegrity(const FString& FilePath, const FString& ExpectedChecksum) const
{
    TArray<uint8> FileData;
    if (FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        FString CalculatedChecksum = FMD5::HashAnsiString((const ANSICHAR*)FileData.GetData(), FileData.Num());
        if (CalculatedChecksum != ExpectedChecksum)
        {
            return false;
        }
    }
    return true;
}

bool UAntiCheatComponent::ProtectMemory(void* MemoryAddress, SIZE_T Size) const
{
    DWORD OldProtection;
    return VirtualProtect(MemoryAddress, Size, PAGE_READWRITE, &OldProtection);
}

bool UAntiCheatComponent::ValidatePlayerPosition(const FVector& ServerPosition, const FVector& ClientPosition, float Tolerance) const
{
    float Distance = FVector::Dist(ServerPosition, ClientPosition);
    if (Distance > Tolerance)
    {
        return false;
    }
    return true;
}

bool UAntiCheatComponent::ValidatePlayerScore(int32 ServerScore, int32 ClientScore) const
{
    if (ClientScore != ServerScore)
    {
        return false;
    }
    return true;
}

void UAntiCheatComponent::LogAntiCheatEvent(const FString& Message) const
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Message);
    }
}

FString UAntiCheatComponent::CalculateFileChecksum(const FString& FilePath) const
{
    TArray<uint8> FileData;
    if (FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        return FMD5::HashAnsiString((const ANSICHAR*)FileData.GetData(), FileData.Num());
    }
    return FString();
}

void UAntiCheatComponent::SendHttpRequest(const FString& Url, const FString& Payload) const
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL(Url);
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(Payload);
    Request->ProcessRequest();
}
