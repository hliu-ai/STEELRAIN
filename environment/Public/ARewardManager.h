// === ARewardManager.h ===

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARewardManager.generated.h"

class AFovealCone;

UCLASS()
class STEELRAIN_H_API ARewardManager : public AActor
{
    GENERATED_BODY()

public:
    ARewardManager();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    void AddHitReward();

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    void AddShotPenalty();

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    void RegisterTarget(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    void UnregisterTarget(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    float GetCurrentReward();

    UFUNCTION(BlueprintCallable, Category = "Rewards")
    void OnEpisodeReset();

    /** Enable detailed logging of reward components each tick */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebug = true; //set to false originally

    /** Returns the DeltaTime of the most recent tick (in seconds) */
    UFUNCTION(BlueprintCallable, Category = "Timing")
    float GetLastTickDeltaTime() const;

protected:
    virtual void BeginPlay() override;

private:
    void ApplyTimePenalty(float DeltaTime);
    void ComputeDistanceShaping();

    // Accumulators
    float PendingReward;
    float LastPendingSnapshot;
    float LastTimePenalty;
    float LastShapingReward;
    float LastDeltaShapingReward;

    // Last tick's DeltaTime
    float LastTickDeltaTime = 0.0f;

    // Configurable reward values
    UPROPERTY(EditAnywhere, Category = "Rewards")
    float HitReward = 250.f;

    UPROPERTY(EditAnywhere, Category = "Rewards")
    float PerShotPenalty = 1.f;

    /** Maximum per-tick shaping reward when aim is perfect */
    UPROPERTY(EditAnywhere, Category = "Rewards")
    float MaxShapingReward = 0.1f;

    /** Exponent applied to normalized distance; higher => stronger incentive for perfect aim */
    UPROPERTY(EditAnywhere, Category = "Rewards")
    float ShapingExponent = 2.0f;

    /** Scale for delta-based shaping reward (per-tick improvement) */
    UPROPERTY(EditAnywhere, Category = "Rewards")
    float DeltaShapingRewardScale = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Rewards")
    float PenaltyPerSecond = 1.f;

    // Sensor reference
    UPROPERTY()
    AFovealCone* FovealConePtr;

    // Only ever one target at a time
    TArray<TWeakObjectPtr<AActor>> ActiveTargets;

    // Last normalized distance, used for delta shaping
    float LastNormDist = 0.0f;
};
