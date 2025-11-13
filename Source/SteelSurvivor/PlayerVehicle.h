#pragma once

#include "CoreMinimal.h"
#include "VehiclePawn.h"
#include "InputActionValue.h"
#include "PlayerVehicle.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class STEELSURVIVOR_API APlayerVehicle : public AVehiclePawn
{
    GENERATED_BODY()

public:
    APlayerVehicle();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// UPROPERTY() ACharacter* StoredCharacter = nullptr; //원래 탑승자 캐릭터 기억

	// bool EnterVehicle(ACharacter* Char, AController* Controller);
	// void ExitVehicle();

protected:

    /* Camera */
    // UPROPERTY(VisibleAnywhere)
    // USpringArmComponent* SpringArm;
    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    /* Enhanced Input */
    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputMappingContext* IMC_Vehicle;
    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_VehicleMove;
    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* IA_VehicleTurn;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* IA_ExitVehicle;

    /* Movement Params */
    UPROPERTY(EditAnywhere, Category="Movement") //3000
    float MaxSpeed = 6000.f;
    UPROPERTY(EditAnywhere, Category="Movement") //1200
    float Acceleration = 2000.f;
    UPROPERTY(EditAnywhere, Category="Movement") //
    float Deceleration = 500.f;
    UPROPERTY(EditAnywhere, Category="Movement") //90
    float TurnRate = 90.f;   // deg/sec

	// 튜닝
	UPROPERTY(EditAnywhere, Category="Vehicle|Params")
	float SteerSmoothIn  = 12.f;    // 키 누를 때 붙는 속도
	UPROPERTY(EditAnywhere, Category="Vehicle|Params")
	float SteerSmoothOut = 6.f;     // 키 뗄 때 돌아오는 속도

    /* Runtime */
    float CurrentSpeed = 0.f;
    float SteeringInput = 0.f;
	float SteeringSmoothed = 0.f; // 보정/회전에 쓰는 스무딩 값

    void Move(const FInputActionValue& Value);
    void Turn(const FInputActionValue& Value);

	void TryExitVehicle();

    // IMC 전환을 안전하게 하려면 PossessedBy/UnPossessed도 오버라이드 가능
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
};
