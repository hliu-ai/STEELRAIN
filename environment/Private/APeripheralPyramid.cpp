// APeripheralPyramid.cpp
#include "APeripheralPyramid.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "URayUtils.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

APeripheralPyramid::APeripheralPyramid()
{
    PrimaryActorTick.bCanEverTick = false; // this was set to false, unsure what true does to it...

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    RootComponent = ArrowComponent;

    PeripheralGridDimension = 27;
    PeripheralPyramidAngleDegrees = 8.0f;
    PeripheralMaxRange = 15000.0f;
    bShowPeripheralRays = false; // CHANGE THIS TO FALSE TO TURN OFF RAYS
}

void APeripheralPyramid::BeginPlay()
{
    Super::BeginPlay();
}

int32 APeripheralPyramid::GetWidth() const
{
    // Aspect ratio 1.5:1 (width : height)
    return FMath::RoundToInt(PeripheralGridDimension * 1.5f);
}

int32 APeripheralPyramid::GetHeight() const
{
    return PeripheralGridDimension;
}

TArray<float> APeripheralPyramid::ComputePeripheralFlags()
{
    const int32 H = GetHeight();
    const int32 W = GetWidth();
    const int32 Total = W * H;
    TArray<float> Flags;
    Flags.Reserve(Total);

    FVector Origin = ArrowComponent->GetComponentLocation();
    FVector Forward = ArrowComponent->GetForwardVector().GetSafeNormal();
    FVector Right = ArrowComponent->GetRightVector().GetSafeNormal();
    FVector Up = ArrowComponent->GetUpVector().GetSafeNormal();

    float MaxAngRad = FMath::DegreesToRadians(PeripheralPyramidAngleDegrees);
    float VertScale = FMath::Tan(MaxAngRad);
    float HorzScale = 1.5f * VertScale;
    float CenterX = (W - 1) * 0.5f;
    float CenterY = (H - 1) * 0.5f;

    UWorld* World = GetWorld();
    if (!World)
    {
        Flags.Init(0.0f, Total);
        return Flags;
    }

    // ROW-MAJOR: for each row (j) then each column (i)
    for (int32 j = 0; j < H; ++j)         // j = 0 -> top
    {
        float NormY = (CenterY - j) / CenterY;
        for (int32 i = 0; i < W; ++i)     // i = 0 -> left
        {
            float NormX = (i - CenterX) / CenterX;

            FVector Perturb = Right * (NormX * HorzScale)
                + Up * (NormY * VertScale);
            FVector Dir = (Forward + Perturb).GetSafeNormal();

            float flag = URayUtils::ComputeRayData(Origin, Dir, PeripheralMaxRange, World).OnTargetInt;
            Flags.Add(flag);

            if (bShowPeripheralRays)
                DrawDebugLine(World, Origin, Origin + Dir * PeripheralMaxRange, FColor::Green, false, 0, 0, 1.5f);
        }
    }

    return Flags;
}
