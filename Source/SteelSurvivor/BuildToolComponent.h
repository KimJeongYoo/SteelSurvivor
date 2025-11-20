#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildToolComponent.generated.h"

class UPlatformGridComponent;
class ABuildableBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEELSURVIVOR_API UBuildToolComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UBuildToolComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 빌드 모드 토글
    void ToggleBuildMode();
    void SetBuildMode(bool bEnable);

    // 실제 건설 실행 (마우스 클릭 등에서 호출)
    void ConfirmBuild();

    UPROPERTY(EditAnywhere, Category="Build")
    TSubclassOf<ABuildableBase> FloorClass;

    // 트레이스 거리
    UPROPERTY(EditAnywhere, Category="Build")
    float TraceDistance = 3000.f;

protected:
    virtual void BeginPlay() override;

private:
	// 망치 들면
    bool bBuildMode = false;

    // 현재 조준 중인 그리드/칸
    UPROPERTY()
    UPlatformGridComponent* CurrentGrid = nullptr;

    int32 PreviewX = -1;
    int32 PreviewY = -1;

    // 미리보기 액터
    UPROPERTY()
    ABuildableBase* PreviewActor = nullptr;

    void UpdatePreview();
};
