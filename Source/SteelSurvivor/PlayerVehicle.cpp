#include "PlayerVehicle.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "InputActionValue.h"

APlayerVehicle::APlayerVehicle()
{
    PrimaryActorTick.bCanEverTick = true;

    // SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    // SpringArm->SetupAttachment(RootComponent);
    // SpringArm->TargetArmLength = 450.f;
    // SpringArm->bUsePawnControlRotation = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
}

void APlayerVehicle::BeginPlay()
{
    Super::BeginPlay();
    // SpringArm->SetWorldRotation(GetActorRotation());
}

void APlayerVehicle::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (APlayerController* PC = Cast<APlayerController>(NewController))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsys =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
            {
                if (IMC_Vehicle)
                {
                    Subsys->AddMappingContext(IMC_Vehicle, /*Priority=*/1);
                }
            }
        }
    }
}

void APlayerVehicle::UnPossessed()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsys =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
            {
                if (IMC_Vehicle)
                {
                    Subsys->RemoveMappingContext(IMC_Vehicle);
                }
            }
        }
    }

    Super::UnPossessed();
}

void APlayerVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_VehicleMove)
        {
            EIC->BindAction(IA_VehicleMove, ETriggerEvent::Triggered, this, &APlayerVehicle::Move);
            EIC->BindAction(IA_VehicleMove, ETriggerEvent::Completed, this, &APlayerVehicle::Move);
        }
        if (IA_VehicleTurn)
        {
            EIC->BindAction(IA_VehicleTurn, ETriggerEvent::Triggered, this, &APlayerVehicle::Turn);
            EIC->BindAction(IA_VehicleTurn, ETriggerEvent::Completed, this, &APlayerVehicle::Turn);
        }
        if (IA_ExitVehicle)
        {
            EIC->BindAction(IA_ExitVehicle, ETriggerEvent::Started, this, &APlayerVehicle::TryExitVehicle);
        }
    }
}

void APlayerVehicle::TryExitVehicle()
{
    const bool bOk = ExitVehicle();  // AVehiclePawn에서 상속 → 직접 호출

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, bOk ? FColor::Green : FColor::Red,
            bOk ? TEXT("ExitVehicle OK") : TEXT("ExitVehicle Fail"));
}




void APlayerVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 스티어 스무딩
    const bool bIncreasing = FMath::Abs(SteeringInput) > FMath::Abs(SteeringSmoothed);
    const float SmoothRate = bIncreasing ? SteerSmoothIn : SteerSmoothOut;
    SteeringSmoothed = FMath::FInterpTo(SteeringSmoothed, SteeringInput, DeltaTime, SmoothRate);

    // 속도 비례 회전
    const float SpeedRatio = FMath::Clamp(FMath::Abs(CurrentSpeed) / MaxSpeed, 0.f, 1.f);
    const float EffectiveTurnRate = TurnRate * SpeedRatio;
    const float YawDelta = SteeringSmoothed * EffectiveTurnRate * DeltaTime;

    // 이동/회전
    const FVector MoveVec = GetActorForwardVector() * CurrentSpeed * DeltaTime;
    AddActorWorldOffset(MoveVec, true);
    AddActorLocalRotation(FRotator(0.f, YawDelta, 0.f));

    // 자연 감속
    CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaTime, 1.0f);
}

void APlayerVehicle::Move(const FInputActionValue& Value)
{
    const float Axis = Value.Get<float>();
    const float dt = GetWorld()->GetDeltaSeconds();

    if (FMath::Abs(Axis) > KINDA_SMALL_NUMBER)
    {
        CurrentSpeed = FMath::Clamp(CurrentSpeed + Axis * Acceleration * dt, -MaxSpeed, MaxSpeed);
    }
    else
    {
        CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, dt, 2.0f);
    }
}

void APlayerVehicle::Turn(const FInputActionValue& Value)
{
    SteeringInput = Value.Get<float>();
}
