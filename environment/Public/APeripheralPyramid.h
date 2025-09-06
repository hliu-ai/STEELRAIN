// APeripheralPyramid.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APeripheralPyramid.generated.h"

UCLASS()
class STEELRAIN_H_API APeripheralPyramid : public AActor
{
    GENERATED_BODY()

public:
    APeripheralPyramid();

protected:
    virtual void BeginPlay() override;

public:
    /**
     * Returns a flattened array of hit flags for the peripheral grid (row-major order).
     * Length = GetWidth() * GetHeight().
     */
    UFUNCTION(BlueprintCallable, Category = "Observation")
    TArray<float> ComputePeripheralFlags();

    /** Width of the peripheral grid (number of columns). */
    UFUNCTION(BlueprintCallable, Category = "Observation")
    int32 GetWidth() const;

    /** Height of the peripheral grid (number of rows). */
    UFUNCTION(BlueprintCallable, Category = "Observation")
    int32 GetHeight() const;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UArrowComponent* ArrowComponent;

    // Number of rows in the grid.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pyramid", meta = (AllowPrivateAccess = "true"))
    int32 PeripheralGridDimension;

    // Maximum angle deviation (in degrees) for rays from forward.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pyramid", meta = (AllowPrivateAccess = "true"))
    float PeripheralPyramidAngleDegrees;

    // Maximum ray length.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pyramid", meta = (AllowPrivateAccess = "true"))
    float PeripheralMaxRange;

    // Whether to draw debug rays.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pyramid", meta = (AllowPrivateAccess = "true"))
    bool bShowPeripheralRays;
};
