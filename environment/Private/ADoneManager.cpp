// ADoneManager.cpp
#include "ADoneManager.h"
#include "Engine/Engine.h"

ADoneManager::ADoneManager()
{
    // No per-frame ticking needed
    PrimaryActorTick.bCanEverTick = false;
    bCurrentDone = false;
    bPrintDebugLogs = false;
}

bool ADoneManager::GetCurrentDone() const
{
    return bCurrentDone;
}

void ADoneManager::SetCurrentDone(bool bDone)
{
    bCurrentDone = bDone;
    if (bPrintDebugLogs)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DoneManager] CurrentDone = %s"), bCurrentDone ? TEXT("True") : TEXT("False"));
    }
}
