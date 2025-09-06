// TCPEnvSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <atomic>
#include <thread>
#include "TCPEnvSubsystem.generated.h"

class FSocket;
class UUE5Game;

/**
 * GameInstanceSubsystem that hosts the TCP environment server
 * and persists across level loads in both PIE and Standalone.
 */
UCLASS()
class STEELRAIN_H_API UTCPEnvSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Blueprint-callable manual reset. */
    UFUNCTION(BlueprintCallable, Category = "RL")
    void ResetEpisode();

private:
    /** Fired on every world initialization—used to catch the PIE or Game world. */
    void OnPostWorldInit(UWorld* World, UWorld::InitializationValues InitValues);

    /** Main loop: bind  listen  accept  serve JSON RPCs  repeat. */
    void ListenerThreadFunc();

    bool SendJson(FSocket* Socket, const TSharedPtr<FJsonObject>& JsonObj);
    TSharedPtr<FJsonObject> ReceiveJson(FSocket* Socket);

    std::thread       ListenerThread;
    std::atomic<bool> bShouldStop{ false };
    int32             Port = 7777;

    FSocket* ListenSocket = nullptr;
    FSocket* ClientSocket = nullptr;
    UUE5Game* Env = nullptr;
};
