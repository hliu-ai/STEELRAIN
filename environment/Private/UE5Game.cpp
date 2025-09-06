// UE5Game.cpp
#include "UE5Game.h"
#include "Kismet/GameplayStatics.h"
#include "AObservationManager.h"
#include "ARewardManager.h"
#include "ADoneManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/PlatformTime.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UUE5Game::UUE5Game()
    : World(nullptr)
    , PC(nullptr)
    , Character(nullptr)
    , MoveComp(nullptr)
    , SpawnLocation(FVector::ZeroVector)
    , SpawnControlRotation(FRotator::ZeroRotator)
    , SpawnActorRotation(FRotator::ZeroRotator)
    , ObservationManager(nullptr)
    , RewardManager(nullptr)
    , DoneManager(nullptr)
    , bManagersBound(false)
{
}

void UUE5Game::Initialize(UWorld* InWorld)
{
    World = InWorld;
    if (!World) return;

    PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    Character = Cast<ACharacter>(PC->GetPawn());
    if (!Character) return;

    MoveComp = Character->GetCharacterMovement();
    Character->bUseControllerRotationYaw = true;
    Character->bUseControllerRotationPitch = false;
    MoveComp->bOrientRotationToMovement = false;

    // record location but ignore stored rotators
    SpawnLocation = Character->GetActorLocation();
}

void UUE5Game::BindManagers()
{
    if (bManagersBound || !World) return;

    ObservationManager = Cast<AObservationManager>(
        UGameplayStatics::GetActorOfClass(World, AObservationManager::StaticClass()));
    RewardManager = Cast<ARewardManager>(
        UGameplayStatics::GetActorOfClass(World, ARewardManager::StaticClass()));
    DoneManager = Cast<ADoneManager>(
        UGameplayStatics::GetActorOfClass(World, ADoneManager::StaticClass()));

    bManagersBound = true;
    UE_LOG(LogTemp, Log, TEXT("UUE5Game: Managers bound: Obs=%s, Rwd=%s, Done=%s"),
        ObservationManager ? TEXT("OK") : TEXT("NULL"),
        RewardManager ? TEXT("OK") : TEXT("NULL"),
        DoneManager ? TEXT("OK") : TEXT("NULL"));
}

FStepResult UUE5Game::Reset()
{
    BindManagers();

    // 1) Clear out reward manager state
    if (RewardManager)
    {
        RewardManager->OnEpisodeReset();
    }

    // 2) Reset pawn position & rotation
    if (PC && Character)
    {
        PC->SetControlRotation(DefaultControlSpawnRotation);
        Character->SetActorLocation(SpawnLocation);
        Character->SetActorRotation(DefaultActorSpawnRotation);
    }

    // 3) Reset done flag
    if (DoneManager)
    {
        DoneManager->SetCurrentDone(false);
    }

    // 4) Build result
    FStepResult Result;
    if (ObservationManager)
    {
        Result.Obs = ObservationManager->GetObservation();
    }
    Result.Reward = 0.0f;
    Result.Done = false;
    Result.DeltaTime = 0.0f;
    return Result;
}

FStepResult UUE5Game::Step(float PitchDelta, float YawDelta, int32 FireFlag)
{
    if (!PC || !Character)
    {
        Initialize(World);
    }
    BindManagers();

    FRotator Curr = PC ? PC->GetControlRotation() : FRotator::ZeroRotator;
    FRotator Raw(Curr.Pitch + PitchDelta,
        Curr.Yaw + YawDelta,
        0.0f);

    float P = FMath::Clamp(Raw.Pitch, MinPitch, MaxPitch);
    float Y = FMath::Clamp(Raw.Yaw, MinYaw, MaxYaw);
    FRotator Clamped(P, Y, 0.0f);

    if (PC && Character)
    {
        PC->SetControlRotation(Clamped);
        Character->SetActorRotation(FRotator(0.0f, Y, 0.0f));
    }

    if (FireFlag != 0)
    {
        Fire();
    }

    FStepResult Result;
    if (ObservationManager)
    {
        Result.Obs = ObservationManager->GetObservation();
    }
    Result.Reward = RewardManager ? RewardManager->GetCurrentReward() : 0.0f;
    Result.Done = DoneManager ? DoneManager->GetCurrentDone() : false;
    if (DoneManager)
    {
        DoneManager->SetCurrentDone(false);
    }
    // 5) Propagate the engine's real DeltaTime
    Result.DeltaTime = RewardManager ? RewardManager->GetLastTickDeltaTime() : 0.0f;

    return Result;
}

void UUE5Game::Fire_Implementation()
{
    double Now = FPlatformTime::Seconds();
    double Interval = 60.0 / MaxRoundsPerMinute;

    if (Now - LastFireTime < Interval)
    {
        UE_LOG(LogTemp, Verbose,
            TEXT("Fire blocked: %.3f/%.3f sec cooldown"),
            Now - LastFireTime, Interval);
        return;
    }

    LastFireTime = Now;

    if (DoneManager)
    {
        DoneManager->FireAction();
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UUE5Game::Fire_Implementation(): DoneManager is null"));
    }
}
