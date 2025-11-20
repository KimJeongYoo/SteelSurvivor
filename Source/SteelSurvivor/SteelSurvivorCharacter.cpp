#include "SteelSurvivorCharacter.h"
#include "PlayerVehicle.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "BuildToolComponent.h"

// Enhanced Input
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASteelSurvivorCharacter::ASteelSurvivorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1인칭용 회전 설정
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;

    // 1인칭 카메라 부착
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // 눈높이 정도
    FirstPersonCamera->bUsePawnControlRotation = true;
    
    BuildTool = CreateDefaultSubobject<UBuildToolComponent>(TEXT("BuildTool"));
}

void ASteelSurvivorCharacter::BeginPlay()
{
    Super::BeginPlay();

}

void ASteelSurvivorCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (APlayerController* PC = Cast<APlayerController>(NewController))
    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    if (auto* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
    {
        Sub->ClearAllMappings();
        if (IMC_Player) Sub->AddMappingContext(IMC_Player, 0);
    }
}

void ASteelSurvivorCharacter::UnPossessed()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    if (ULocalPlayer* LP = PC->GetLocalPlayer())
    if (auto* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
    {
        Sub->RemoveMappingContext(IMC_Player);
    }
    Super::UnPossessed();
}


void ASteelSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)
        {
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASteelSurvivorCharacter::Move);
            EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &ASteelSurvivorCharacter::Move);
        }

        if (IA_Look)
        {
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASteelSurvivorCharacter::Look);
        }

		if (IA_EnterVehicle)
		{
			EIC->BindAction(IA_EnterVehicle, ETriggerEvent::Started, this, &ASteelSurvivorCharacter::TryEnterNearestVehicle);
		}

        if (IA_BuildMode)
        {
            EIC->BindAction(IA_BuildMode, ETriggerEvent::Started, this, &ASteelSurvivorCharacter::ToggleBuildMode);
        }

        if (IA_BuildConfirm)
        {
            EIC->BindAction(IA_BuildConfirm, ETriggerEvent::Started, this, &ASteelSurvivorCharacter::ConfirmBuild);
        }
    }
}

void ASteelSurvivorCharacter::ToggleBuildMode()
{
    if (BuildTool)
        BuildTool->ToggleBuildMode();
}

void ASteelSurvivorCharacter::ConfirmBuild()
{
    if (BuildTool)
        BuildTool->ConfirmBuild();
}

void ASteelSurvivorCharacter::Move(const FInputActionValue& Value)
{
    const FVector2D InputVec = Value.Get<FVector2D>();

    if (Controller && (InputVec.SizeSquared() > KINDA_SMALL_NUMBER))
    {
        const FRotator ControlRot = Controller->GetControlRotation();
        const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

        const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector RightDir   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDir, InputVec.Y);
        AddMovementInput(RightDir,   InputVec.X);

        auto* CMC = GetCharacterMovement();
        CMC->SetPlaneConstraintEnabled(false);   // XY 강제 평면 금지 (FloorCheck 깨질 수 있음)
        CMC->MaxStepHeight = 45.f;
        CMC->SetWalkableFloorAngle(46.f);
        CMC->PerchAdditionalHeight = 20.f;
    }
}

void ASteelSurvivorCharacter::Look(const FInputActionValue& Value)
{
    const FVector2D LookVec = Value.Get<FVector2D>();

    AddControllerYawInput(LookVec.X * YawSensitivity);
    AddControllerPitchInput(LookVec.Y * PitchSensitivity); //Roll Scale 60%
}


void ASteelSurvivorCharacter::TryEnterNearestVehicle()
{
    // 카메라 기준 전방으로 짧게 스윕하여 차량 찾기
    const FVector Start = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation();
    const FVector End   = Start + (FirstPersonCamera ? FirstPersonCamera->GetForwardVector() : GetActorForwardVector()) * EnterTraceDistance;

    // 캡슐 스윕(좁은 폭으로 앞을 훑어보기)
    FHitResult Hit;
    FCollisionShape Shape = FCollisionShape::MakeSphere(EnterSweepRadius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EnterVehicle), false, this);

    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, Start, End, FQuat::Identity, ECC_Visibility, Shape, Params);

    AVehiclePawn* Vehicle = bHit ? Cast<AVehiclePawn>(Hit.GetActor()) : nullptr;

    if (!Vehicle)
    {
        // 한 번 더: 정확히 정면만 라인트레이스
        GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
        Vehicle = Cast<AVehiclePawn>(Hit.GetActor());
    }

    if (Vehicle)
    {
        if (AController* Ctrl = GetController())
        {
            if (Vehicle->EnterVehicle(this, Ctrl))
            {
                if (GEngine)
                    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("EnterVehicle OK"));
                return;
            }
        }
    }

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No vehicle or EnterVehicle failed"));
}
