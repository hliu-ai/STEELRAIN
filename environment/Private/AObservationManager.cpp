// === AObservationManager.cpp ===

#include "AObservationManager.h"
#include "APeripheralPyramid.h"
#include "AFovealCone.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"

AObservationManager::AObservationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bShowGridDebug = false;
}

void AObservationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponents();

    // Bind M key to snapshot
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        EnableInput(PC);
        if (InputComponent)
        {
            InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AObservationManager::SnapshotObservation);
            if (bShowGridDebug && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Bound M to SnapshotObservation"));
            }
        }
    }
}

void AObservationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    GenerateObservation();
    //GetObservation(); //comment out when done debugging AND following if statement

    //if (bShowGridDebug && GEngine)
    //{
     //   GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan,
      //      FString::Printf(TEXT("Obs len=%d"), PeripheralFlags.Num() + 5));
    //}
}

void AObservationManager::InitializeComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<APeripheralPyramid> It(World); It; ++It)
    {
        PeripheralPyramid = *It;
        break;
    }
    for (TActorIterator<AFovealCone> It(World); It; ++It)
    {
        FovealCone = *It;
        break;
    }
    if ((!PeripheralPyramid || !FovealCone) && bShowGridDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("ObservationManager: Missing sensors"));
    }
}

void AObservationManager::GenerateObservation()
{
    if (!PeripheralPyramid || !FovealCone) return;
    PeripheralFlags = PeripheralPyramid->ComputePeripheralFlags();
}

TArray<float> AObservationManager::GetObservation() const
{
    // Prepare output array
    TArray<float> Combined;
    Combined.Reserve(PeripheralFlags.Num() + 5);
    Combined.Append(PeripheralFlags);

    // 1) Read and normalize controller rotation
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FRotator R = PC->GetControlRotation();
        Pitch = R.Pitch;
        Yaw = R.Yaw;
    }
    float NormPitch = FMath::Clamp((Pitch - 4.0f) / (21.0f - 4.0f), 0.0f, 1.0f);
    float NormYaw = FMath::Clamp((Yaw - 253.0f) / (278.0f - 253.0f), 0.0f, 1.0f);
    Combined.Add(NormPitch);
    Combined.Add(NormYaw);

    // 2) Prepare for spatial metrics
    const FVector Origin = FovealCone->GetConeOrigin();
    const FVector Forward = FovealCone->GetConeForward();

    // 3) Find the target actor
    AActor* Target = nullptr;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(TEXT("Target")))
        {
            Target = *It;
            break;
        }
    }

    // 4) Compute shared distance, cosine, and overlap
    float NormDist = 0.0f;
    float SignedNormAngle = 0.0f;
    float Overlap = 0.0f;

    if (Target)
    {
        // a) Normalized distance via shared helper
        NormDist = FovealCone->GetNormalizedDistanceToTarget(Target);

        // b) Signed normalized angle on XZ plane
        SignedNormAngle = FovealCone->GetNormalizedSignedAngleToTarget(Target);

        // c) Overlap flag via line trace
        FHitResult Hit;
        FCollisionQueryParams Params;
        if (AActor* OwnerActor = GetOwner())
        {
            Params.AddIgnoredActor(OwnerActor);
        }
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            Origin,
            Origin + Forward * 10000.0f,
            ECC_Visibility,
            Params
        );
        if (bHit && Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("Target")))
        {
            Overlap = 1.0f;
        }
    }

    // 5) Append to observation
    Combined.Add(NormDist);
    Combined.Add(SignedNormAngle);
    Combined.Add(Overlap);

    // 6) Debug log
    //UE_LOG(LogTemp, Log,TEXT("[Obs] NormPitch=%.3f NormYaw=%.3f NormDist=%.3f SignedAngle=%.3f Overlap=%.1f"),NormPitch, NormYaw, NormDist, SignedNormAngle, Overlap);


    return Combined;
}


void AObservationManager::SnapshotObservation()
{
    TArray<float> Obs = GetObservation();
    if (Obs.Num() < PeriphRows * PeriphCols + 5) return;

    FString Text = TEXT("--- Observation Snapshot ---\n");

    // Grid
    for (int32 r = 0; r < PeriphRows; ++r)
    {
        for (int32 c = 0; c < PeriphCols; ++c)
        {
            float v = Obs[r * PeriphCols + c];
            Text += FString::Printf(TEXT("%d "), v > 0.5f ? 1 : 0);
        }
        Text += TEXT("\n");
    }

    int32 base = PeriphRows * PeriphCols;
    Text += FString::Printf(TEXT("NormPitch: %.3f\n"), Obs[base + 0]);
    Text += FString::Printf(TEXT("NormYaw:   %.3f\n"), Obs[base + 1]);
    Text += FString::Printf(TEXT("NormDist:  %.3f\n"), Obs[base + 2]);
    Text += FString::Printf(TEXT("CosAngle:  %.3f\n"), Obs[base + 3]);
    Text += FString::Printf(TEXT("Overlap:   %.1f\n"), Obs[base + 4]);

    // Log and save
    UE_LOG(LogTemp, Display, TEXT("%s"), *Text);
    FString Path = FPaths::ProjectSavedDir() + TEXT("ObsSnap_") + FDateTime::Now().ToString() + TEXT(".txt");
    FFileHelper::SaveStringToFile(Text, *Path);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("Obs snapshot saved to %s"), *Path));
    }
}
