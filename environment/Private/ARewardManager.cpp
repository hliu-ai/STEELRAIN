// === ARewardManager.cpp ===

#include "ARewardManager.h"
#include "AFovealCone.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

ARewardManager::ARewardManager()
    : PendingReward(0.f)
    , LastPendingSnapshot(0.f)
    , LastTimePenalty(0.f)
    , LastShapingReward(0.f)
    , LastDeltaShapingReward(0.f)
    , LastTickDeltaTime(0.f)
{
    PrimaryActorTick.bCanEverTick = true;
}

void ARewardManager::BeginPlay()
{
    Super::BeginPlay();

    // Find the one FovealCone in the world
    for (TActorIterator<AFovealCone> It(GetWorld()); It; ++It)
    {
        FovealConePtr = *It;
        break;
    }
    if (!FovealConePtr && bShowDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RewardManager] No FovealCone found; shaping disabled."));
    }
}

void ARewardManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Record the real tick DeltaTime
    LastTickDeltaTime = DeltaTime;

    // Reset this tick's components
    LastTimePenalty = 0.f;
    LastShapingReward = 0.f;
    LastDeltaShapingReward = 0.f;

    // Apply time penalty and shaping
    ApplyTimePenalty(DeltaTime);
    ComputeDistanceShaping();

    // Debug breakdown
    if (bShowDebug && GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT(
            "[RewardManager] DeltaTime=%.4f  TimePenalty=%.4f  Shaping=%.4f  DeltaShaping=%.4f  Pending=%.4f"),
            LastTickDeltaTime, LastTimePenalty, LastShapingReward, LastDeltaShapingReward, PendingReward
        );
    }
}

void ARewardManager::ApplyTimePenalty(float DeltaTime)
{
    if (PenaltyPerSecond <= 0.f) return;
    LastTimePenalty = PenaltyPerSecond * DeltaTime;
    PendingReward -= LastTimePenalty;
}

void ARewardManager::ComputeDistanceShaping()
{
    if (!FovealConePtr) return;

    // Ensure exactly one valid target
    ActiveTargets.RemoveAll([](auto& Ptr) { return !Ptr.IsValid(); });
    if (ActiveTargets.Num() != 1) return;

    AActor* Target = ActiveTargets[0].Get();
    if (!Target) return;

    // 1-4) Single-source NormDist
    float NormDist = FovealConePtr->GetNormalizedDistanceToTarget(Target);

    // 5) Exponential scaling to MaxShapingReward
    LastShapingReward = MaxShapingReward * FMath::Pow(NormDist, ShapingExponent);
    PendingReward += LastShapingReward;

    // 6) Delta-based shaping reward: reward any improvement in normalized distance, currently unused but leave the scaffolding in.
    if (DeltaShapingRewardScale != 0.0f)
    {
        float delta = NormDist - LastNormDist;
        float positiveDelta = FMath::Max(0.f, delta);
        LastDeltaShapingReward = DeltaShapingRewardScale * positiveDelta;
        PendingReward += LastDeltaShapingReward;
    }
    else
    {
        LastDeltaShapingReward = 0.f;
    }
    LastNormDist = NormDist;
}

void ARewardManager::AddHitReward()
{
    PendingReward += HitReward;
    if (bShowDebug && GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT("[RewardManager] HitReward=+%.1f Pending=%.4f"), HitReward, PendingReward);
    }
}

void ARewardManager::AddShotPenalty()
{
    PendingReward -= PerShotPenalty;
    if (bShowDebug && GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT("[RewardManager] ShotPenalty=-%.1f Pending=%.4f"), PerShotPenalty, PendingReward);
    }
}

void ARewardManager::RegisterTarget(AActor* TargetActor)
{
    ActiveTargets.AddUnique(TargetActor);
}

void ARewardManager::UnregisterTarget(AActor* TargetActor)
{
    ActiveTargets.Remove(TargetActor);
}

float ARewardManager::GetCurrentReward()
{
    float Delta = PendingReward - LastPendingSnapshot;
    LastPendingSnapshot = PendingReward;
    return Delta;
}

void ARewardManager::OnEpisodeReset()
{
    PendingReward = 0.f;
    LastPendingSnapshot = 0.f;
    LastNormDist = 0.0f;
}

// Getter for the most recent DeltaTime
float ARewardManager::GetLastTickDeltaTime() const
{
    return LastTickDeltaTime;
}
