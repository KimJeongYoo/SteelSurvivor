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

    // === 지면 추종(Z 및 Pitch/Roll 보정) ===
    // ApplyGroundFollow(DeltaTime);

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

// void APlayerVehicle::ApplyGroundFollow(float DeltaTime)
// {
//     UWorld* World = GetWorld();
//     if (!World) return;

//     const FVector ActorLoc = GetActorLocation();
//     const FVector Up = FVector::UpVector;

//     const FVector TraceStart = ActorLoc + Up * TraceUp;
//     const FVector TraceEnd   = ActorLoc - Up * TraceDown;

//     FHitResult Hit;
//     FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerVehicle_FloorTrace), false, this);

//     // 지면 트레이스(Visibility 또는 WorldStatic 사용)
//     const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
//     // DrawDebugLine(World, TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.f);

//     if (!bHit)
//     {
//         // 지면을 못 찾으면 천천히 아래로 끌어내려 낙하 느낌(원하면 중력값 더)
//         const float FallSpeed = 300.f;
//         AddActorWorldOffset(-Up * FallSpeed * DeltaTime, true);
//         return;
//     }

//     const FVector GroundNormal = Hit.ImpactNormal.GetSafeNormal();
//     const float SlopeDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, Up)));

//     // 너무 가파르면(계단이나 벽) 붙지 않고 슬라이드/회피
//     if (SlopeDeg > MaxWalkableSlopeDeg)
//     {
//         // 살짝 위로 들어 올려서 끼임 방지(옵션)
//         AddActorWorldOffset(Up * 2.f, true);
//         return;
//     }

//     // === Z 위치 보정: 지면 + RideHeight ===
//     const float TargetZ = Hit.ImpactPoint.Z + RideHeight;
//     FVector TargetLoc = ActorLoc;
//     TargetLoc.Z = FMath::FInterpTo(ActorLoc.Z, TargetZ, DeltaTime, ZInterpSpeed);

//     // 스윕해서 위치 이동(끼임 방지)
//     SetActorLocation(TargetLoc, /*bSweep=*/true);

//     // === 회전(Pitch/Roll) 지면 정렬 ===
//     if (bAlignToGroundNormal)
//     {
//         // 전진 방향을 지면 위로 투영해 "앞"을 유지
//         const FVector Fwd = GetActorForwardVector();
//         FVector FwdOnPlane = (Fwd - FVector::DotProduct(Fwd, GroundNormal) * GroundNormal).GetSafeNormal();
//         if (FwdOnPlane.IsNearlyZero())
//         {
//             FwdOnPlane = Fwd; // 예외 처리
//         }

//         const FRotator DesiredRot = FRotationMatrix::MakeFromXZ(FwdOnPlane, GroundNormal).Rotator();

//         // Yaw는 우리가 이미 조향했으니 Pitch/Roll만 부드럽게 보정
//         FRotator Current = GetActorRotation();
//         FRotator Target  = FRotator(DesiredRot.Pitch, Current.Yaw, DesiredRot.Roll);

//         const FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaTime, RotInterpSpeed);
//         SetActorRotation(NewRot);
//     }
// }