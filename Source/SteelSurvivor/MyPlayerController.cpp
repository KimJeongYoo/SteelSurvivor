// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "VehiclePawn.h"

// AMyPlayerController.cpp
void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ULocalPlayer* LP = GetLocalPlayer())
        if (auto* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Sub->ClearAllMappings();
            Sub->AddMappingContext( Cast<AVehiclePawn>(InPawn) ? IMC_Vehicle : IMC_Player, 0 );
        }
}


void AMyPlayerController::ApplyIMC(UInputMappingContext* IMC)
{
    if (!IMC) return;
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (auto* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsys->ClearAllMappings();
            Subsys->AddMappingContext(IMC, 0);
        }
    }
}

