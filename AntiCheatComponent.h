#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Http.h"
#include "Json.h"
#include "jwt-cpp/jwt.h"
#include <Windows.h>
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

/**
 * AntiCheat component responsible for detecting and preventing cheating in the game.
 */
class UAntiCheatComponent : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Default constructor for UAntiCheatComponent.
     */
    UAntiCheatComponent();

    /**
     * Check if a debugger is present.
     *
     * @return true if a debugger is detected, false otherwise.
     */
    bool IsDebuggerPresent();

    /**
     * Check if the game is running inside a virtual machine.
     *
     * @return true if running inside a virtual machine, false otherwise.
     */
    bool IsInsideVirtualMachine();

    /**
     * Scan for known hooking libraries in the process.
     *
     * @return true if a hooking library is detected, false otherwise.
     */
    bool ScanForHooks();

    /**
     * Validate game time against server time to detect potential time manipulation.
     */
    void ValidateGameTime();

    /**
     * Validate player data by sending it to the server for verification.
     *
     * @param PlayerId The ID of the player to validate.
     * @param Speed The player's speed value.
     * @param Visibility The player's visibility value.
     * @param Accuracy The player's accuracy value.
     */
    void ValidatePlayerData(FString PlayerId, float Speed, float Visibility, float Accuracy);

    /**
     * Callback function to handle the server validation response.
     *
     * @param Request The HTTP request that was sent.
     * @param Response The HTTP response received from the server.
     * @param bSuccess Indicates whether the request was successful.
     */
    void OnValidateResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

    /**
     * Generate a JWT token for authentication.
     *
     * @return The generated JWT token.
     */
    FString GenerateJWTToken();

    /**
     * Scan for known DLLs in the process memory.
     *
     * @return true if a known DLL is detected, false otherwise.
     */
    bool ScanForDLLs();

    /**
     * Add a player to the ban list and kick them from the game.
     *
     * @param PlayerId The ID of the player to ban.
     */
    void banPlayer(FString PlayerId);

    /**
     * Kick a player from the game.
     *
     * @param PlayerId The ID of the player to kick.
     */
    void KickPlayer(FString PlayerId);

    /**
     * Check the integrity of a game file against a provided checksum.
     *
     * @param FilePath The path to the game file to check.
     * @param ExpectedChecksum The expected checksum of the game file.
     * @return true if the file integrity is valid, false otherwise.
     */
    bool CheckFileIntegrity(FString FilePath, FString ExpectedChecksum);

    /**
     * Protect a region of memory from being modified.
     *
     * @param MemoryAddress The starting address of the memory region to protect.
     * @param Size The size of the memory region to protect.
     * @return true if the memory protection is successful, false otherwise.
     */
    bool ProtectMemory(void* MemoryAddress, SIZE_T Size);

    /**
     * Validate the player's position against the server's position.
     *
     * @param ServerPosition The position of the player according to the server.
     * @param ClientPosition The position of the player according to the client.
     * @param Tolerance The maximum allowable difference between positions.
     * @return true if the player's position is valid, false otherwise.
     */
    bool ValidatePlayerPosition(FVector ServerPosition, FVector ClientPosition, float Tolerance);

    /**
     * Validate the player's score against the server's score.
     *
     * @param ServerScore The score of the player according to the server.
     * @param ClientScore The score of the player according to the client.
     * @return true if the player's score is valid, false otherwise.
     */
    bool ValidatePlayerScore(int32 ServerScore, int32 ClientScore);

private:
    /**
     * List of banned player IDs.
     */
    TArray<FString> banList;

    /**
     * Path to the game file being validated.
     */
    FString GameFilePath;

    /**
     * Expected checksum of the game file being validated.
     */
    FString ExpectedChecksum;

    /**
     * Kick a player from the game after they have been banned.
     *
     * @param PlayerController The player controller of the player to kick.
     */
    void KickPlayerAfterBan(APlayerController* PlayerController);
};
