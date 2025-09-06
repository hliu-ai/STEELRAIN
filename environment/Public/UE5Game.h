// UE5Game.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UE5Game.generated.h"

class UWorld;
class APlayerController;
class ACharacter;
class UCharacterMovementComponent;
class AObservationManager;
class ARewardManager;
class ADoneManager;

USTRUCT(BlueprintType)
struct FStepResult
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<float> Obs;

    UPROPERTY()
    float Reward;

    UPROPERTY()
    bool Done;

    // DeltaTime of the tick (0.0 on reset)
    UPROPERTY()
    float DeltaTime;
};

UCLASS(Blueprintable)
class STEELRAIN_H_API UUE5Game : public UObject
{
    GENERATED_BODY()

public:
    UUE5Game();

    /** Initialize with the current world */
    void Initialize(UWorld* InWorld);

    /** Bind observation/reward/done managers once */
    void BindManagers();

    /** Reset the environment state and return initial observation */
    FStepResult Reset();

    /** Step the environment with pitch, yaw, fire_flag */
    FStepResult Step(float PitchDelta, float YawDelta, int32 FireFlag);

    /** Called by Step() when the fire flag is on */
    UFUNCTION(BlueprintNativeEvent, Category = "Agent|Actions")
    void Fire();
    virtual void Fire_Implementation();

    /** Aim boundaries (degrees) */
    UPROPERTY(EditAnywhere, Category = "Agent|Boundaries")
    float MinYaw = 253.0f;
    UPROPERTY(EditAnywhere, Category = "Agent|Boundaries")
    float MaxYaw = 278.0f;
    UPROPERTY(EditAnywhere, Category = "Agent|Boundaries")
    float MinPitch = 4.0f;
    UPROPERTY(EditAnywhere, Category = "Agent|Boundaries")
    float MaxPitch = 21.0f;

    /** Default spawn rotations (can tweak before build) */
    UPROPERTY(EditAnywhere, Category = "Agent|Spawn")
    FRotator DefaultControlSpawnRotation = FRotator(13.0f, 265.0f, 0.0f);
    UPROPERTY(EditAnywhere, Category = "Agent|Spawn")
    FRotator DefaultActorSpawnRotation = FRotator(0.0f, 265.0f, 0.0f);

private:
    UWorld* World = nullptr;
    APlayerController* PC = nullptr;
    ACharacter* Character = nullptr;
    UCharacterMovementComponent* MoveComp = nullptr;

    FVector   SpawnLocation = FVector::ZeroVector;
    // These initial rotators are now ignored; Reset() uses the defaults
    FRotator  SpawnControlRotation = FRotator::ZeroRotator;
    FRotator  SpawnActorRotation = FRotator::ZeroRotator;

    AObservationManager* ObservationManager = nullptr;
    ARewardManager* RewardManager = nullptr;
    ADoneManager* DoneManager = nullptr;

    // rounds per minute cap (not exposed to editor)
    float MaxRoundsPerMinute = 840.0f;
    // wall-clock timestamp of last shot
    double LastFireTime = 0.0;

    bool bManagersBound = false;
};
