#pragma once

#include "CoreMinimal.h"
#include "BasePawn.h"
#include "VehiclePawn.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class ACharacter;
class AController;

UCLASS()
class STEELSURVIVOR_API AVehiclePawn : public ABasePawn
{
    GENERATED_BODY()

public:
    AVehiclePawn();

    // 운전석 기준점(소켓 대용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    USceneComponent* DriverSeat;

    UFUNCTION(BlueprintCallable)
    bool EnterVehicle(ACharacter* Char, AController* Ctrl);

    UFUNCTION(BlueprintCallable)
    bool ExitVehicle();

    virtual void BeginPlay() override;

protected:
    // 파생(APlayerVehicle)에서도 접근 필요할 수 있어 protected로
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USkeletalMeshComponent* VehicleBodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USceneComponent* WheelControlPoint;

    UPROPERTY()
    ACharacter* SeatedChar = nullptr;
};
