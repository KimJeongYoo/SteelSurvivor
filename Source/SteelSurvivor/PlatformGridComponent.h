#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PlatformGridComponent.generated.h"

class ABuildableBase;
class USceneComponent;

USTRUCT(BlueprintType)
struct FPlatformCell
{
    GENERATED_BODY()

    UPROPERTY()
    bool bOccupied = false;

    UPROPERTY()
    ABuildableBase* OccupiedActor = nullptr;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEELSURVIVOR_API UPlatformGridComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPlatformGridComponent();

	//TestGrid
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // **추가:** 디버그 드로잉을 위한 테스트 함수 선언
    void TestGridDrawing();

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
    // USceneComponent* PlatformOrigin;

    // 최대 폭: 좌우 5칸
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid")
    int32 MaxWidth = 5;          // Y 방향 (좌우)
    // 길이: 앞뒤 7칸
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid")
    int32 Length = 7;            // X 방향 (앞뒤)
    // 시작 시 활성 폭 (가운데 3칸만 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid")
    int32 InitialActiveWidth = 3;
    // 한 칸 크기 (언리얼 유닛, 100 = 1m)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grid")
    float CellSize = 100.f;

    // 어떤 열(폭)이 열려있는지 (왼 → 오른 순서)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
    TArray<bool> ColumnUnlocked;

    // 월드 → 그리드 좌표 변환
    bool WorldToGrid(const FVector& WorldPos, int32& OutX, int32& OutY) const;

    // 그리드 → 월드 좌표 변환 (셀 중심)
    FVector GridToWorld(int32 X, int32 Y) const;

    // 해당 칸에 건설 가능한지?
    bool CanBuildAt(int32 X, int32 Y) const;

    // 셀에 액터 등록/해제
    void SetCell(int32 X, int32 Y, ABuildableBase* Actor);

    // 좌/우 한 칸씩 확장 (나중에 쓸 용도)
    bool UnlockLeftColumn();
    bool UnlockRightColumn();

    // 전체 무게 계산 (나중에 차량 무게에 반영)
    float GetTotalWeight() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<FPlatformCell> Cells;

    int32 Index(int32 X, int32 Y) const { return Y * Length + X; }

    // 현재 활성 폭 (ColumnUnlocked에서 true 갯수)
    int32 GetActiveWidth() const;
};
