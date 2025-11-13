#include "VehiclePawn.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

AVehiclePawn::AVehiclePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    VehicleBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleBodyMesh"));
    VehicleBodyMesh->SetupAttachment(RootComponent);

    WheelControlPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WheelControlPoint"));
    WheelControlPoint->SetupAttachment(RootComponent);

    DriverSeat = CreateDefaultSubobject<USceneComponent>(TEXT("DriverSeat"));
    DriverSeat->SetupAttachment(RootComponent);

}

void AVehiclePawn::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Display, TEXT("Your message"));
}

bool AVehiclePawn::EnterVehicle(ACharacter* Char, AController* Ctrl)
{
    if (!IsValid(Char) || !IsValid(Ctrl) || !IsValid(DriverSeat)) return false;

    UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
    UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    if (!IsValid(Capsule) || !IsValid(Move)) return false;

    // DriverSeat는 Movable
    if (DriverSeat->Mobility != EComponentMobility::Movable)
        DriverSeat->SetMobility(EComponentMobility::Movable);

    // 1) 이동/충돌 안정화
    Move->StopMovementImmediately();
    Move->SetMovementMode(MOVE_None);                         // DisableMovement() 대신 이게 덜 튐
    Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 2) 좌석 월드 TM 미리 얻고, 먼저 맞춤
    const FTransform SeatTM = DriverSeat->GetComponentTransform();
    Char->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Char->SetActorTransform(SeatTM, false, nullptr, ETeleportType::TeleportPhysics);

    // 3) KeepWorld로 부착 (여기서 끝)
    Char->AttachToComponent(DriverSeat, FAttachmentTransformRules::KeepWorldTransform);

    if (APlayerController* PC = Cast<APlayerController>(Ctrl))
    {
        // Char->DisableInput(PC);                    // 입력 잠깐 차단
        PC->SetControlRotation(GetActorRotation()); // 시점 정렬(옵션)
    }

    Ctrl->Possess(this); // ← 여기서 멈춘다면 2)~3)은 OK, 소유권 전환/IMC 쪽 문제
    SeatedChar = Char;   // ← 이제 GC에 안전 (UPROPERTY 덕분)

    return true;
}

bool AVehiclePawn::ExitVehicle()
{
    if (!SeatedChar) return false;

    AController* Ctrl = GetController();
    APlayerController* PC = Cast<APlayerController>(Ctrl);
    if (!PC) return false;

    // 하차 위치(끼임 방지 약간 보정)
    const FVector ExitLoc =
        GetActorLocation()
      + GetActorRightVector() * -200.f
      + GetActorForwardVector() * 40.f
      + FVector(0, 0, 110.f);
    const FRotator ExitRot = GetActorRotation();

    // 분리 및 위치 세팅
    SeatedChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SeatedChar->SetActorLocationAndRotation(ExitLoc, ExitRot, false, nullptr, ETeleportType::TeleportPhysics);

    // 충돌/이동 복구
    if (UCapsuleComponent* Capsule = SeatedChar->GetCapsuleComponent())
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    if (UCharacterMovementComponent* Move = SeatedChar->GetCharacterMovement())
        Move->SetMovementMode(MOVE_Walking);

    // 소유권 반환
    PC->Possess(SeatedChar);

    SeatedChar = nullptr;
    return true;
}
