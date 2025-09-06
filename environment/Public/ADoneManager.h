// ADoneManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADoneManager.generated.h"

/**
 * ADoneManager
 * Manages the 'done' state (episode termination flag) for RL environment integration.
 *
 * Blueprint integration:
 * - Call SetCurrentDone(true) when the episode resets.
 * - Call SetCurrentDone(false) on non-reset ticks.
 * - Use GetCurrentDone() to retrieve the flag in your UE5Game API wrapper.
 * - Toggle bPrintDebugLogs to enable per-call LogTemp output.
 */
UCLASS()
class STEELRAIN_H_API ADoneManager : public AActor
{
    GENERATED_BODY()

public:
    ADoneManager();

    /** Returns whether the current tick is terminal (episode reset). */
    UFUNCTION(BlueprintCallable, Category = "RL")
    bool GetCurrentDone() const;

    /** Sets the terminal flag for this tick; logs if bPrintDebugLogs is true. */
    UFUNCTION(BlueprintCallable, Category = "RL")
    void SetCurrentDone(bool bDone);

    /** Enable to print debug logs on SetCurrentDone calls. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RL")
    bool bPrintDebugLogs;

    // just putting this here, fire flag needs to be inside of an actor and i didnt want to create a whole action manager for this.
    UFUNCTION(BlueprintImplementableEvent, Category = "Agent|Actions")
    void FireAction();

protected:
    /** Tracks whether the episode reset occurred this tick. */
    UPROPERTY(BlueprintReadOnly, Category = "RL")
    bool bCurrentDone;
};