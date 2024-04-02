#include "AntiCheatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "jwt-cpp/jwt.h"  // Include the jwt-cpp library header

UAntiCheatComponent::UAntiCheatComponent()
{
    // Constructor
}
void UAntiCheatComponent::ValidatePlayerData(FString PlayerId, float Speed, float Visibility, float Accuracy)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("POST");
    Request->SetURL("http://your_server_ip:5000/validate");  // Replace with your server IP address
    Request->SetHeader("Content-Type", "application/json");

    FString JsonPayload = FString::Printf(TEXT("{\"player_id\": \"%s\", \"speed\": %f, \"visibility\": %f, \"accuracy\": %f}"), *PlayerId, Speed, Visibility, Accuracy);
    Request->SetContentAsString(JsonPayload);

    Request->OnProcessRequestComplete().BindUObject(this, &UAntiCheatComponent::OnValidateResponseReceived);

    Request->ProcessRequest();
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
