#pragma once

#include "InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SteelSurvivorCharacter.generated.h"



class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UBuildToolComponent;

UCLASS()
class STEELSURVIVOR_API ASteelSurvivorCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASteelSurvivorCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Build")
    UBuildToolComponent* BuildTool;

    void ToggleBuildMode();
    void ConfirmBuild();


    // 인벤토리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    UInventoryComponent* InventoryComp;

    // 인벤토리 위젯
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY()
    UUserWidget* InventoryWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    bool bInventoryOpen = false;

    // 인벤토리 토글 함수
    UFUNCTION()
    void ToggleInventory();

    // 숫자키로 퀵바 선택 (1~9)
    void SelectHotbarIndex(int32 Index);

protected:
    // === 1인칭 카메라 ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent* FirstPersonCamera;

    // === Enhanced Input (에디터에서 할당) ===
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputMappingContext* IMC_Player;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_Move;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_EnterVehicle; // F키 같은 디지털 액션

    //임의로
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_BuildMode; // B키

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_BuildConfirm; // 좌클릭키

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_ToggleInventory; // E
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_SelectSlot1; // 1

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    UInputAction* IA_SelectSlot2; // 2

    // === 입력 처리 함수 ===
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // 감도
    UPROPERTY(EditAnywhere, Category="Input|Sensitivity")
    float YawSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, Category="Input|Sensitivity")
    float PitchSensitivity = 1.0f;

	// 탑승 시도(캐릭터가 호출)
    void TryEnterNearestVehicle();


    // 탐색 거리/반지름(원하면 에디터에서 조절)
    UPROPERTY(EditAnywhere, Category="Interact")
    float EnterTraceDistance = 400.f;

    UPROPERTY(EditAnywhere, Category="Interact")
    float EnterSweepRadius = 60.f;

    // IMC 전환을 안전하게 하려면 PossessedBy/UnPossessed도 오버라이드 가능
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
};
