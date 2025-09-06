// === AObservationManager.h ===
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AObservationManager.generated.h"

class APeripheralPyramid;
class AFovealCone;

UCLASS()
class STEELRAIN_H_API AObservationManager : public AActor
{
    GENERATED_BODY()

public:
    AObservationManager();
    virtual void Tick(float DeltaTime) override;

    /** Fetch the combined observation vector */
    TArray<float> GetObservation() const;

    /** Print & save the current observation (M key) */
    UFUNCTION()
    void SnapshotObservation();

    /** Toggle on-screen basic info each tick */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowGridDebug = false;

protected:
    virtual void BeginPlay() override;

private:
    void InitializeComponents();
    void GenerateObservation();

    UPROPERTY()
    APeripheralPyramid* PeripheralPyramid;

    UPROPERTY()
    AFovealCone* FovealCone;

    TArray<float> PeripheralFlags;

    // Hardcoded grid dimensions
    static constexpr int32 PeriphRows = 27;
    static constexpr int32 PeriphCols = 41;
};
