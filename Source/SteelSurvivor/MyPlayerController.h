// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"


UCLASS()
class AMyPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category="Input")
    class UInputMappingContext* IMC_Player;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    class UInputMappingContext* IMC_Vehicle;

protected:
    virtual void OnPossess(APawn* InPawn) override;
    // virtual void OnUnPossess() override {}
    void ApplyIMC(UInputMappingContext* IMC);
};

