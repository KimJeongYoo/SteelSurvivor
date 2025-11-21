// InventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    // 비어있으면 ItemId == NAME_None 로 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    bool IsEmpty() const { return ItemId.IsNone() || Count <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEELSURVIVOR_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // --- 인벤토리 데이터 ---

    // E 로 여는 5x2 인벤토리 (총 10칸)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 MainCols = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 MainRows = 2;

    // 메인 인벤토리 슬롯 (5x2)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TArray<FInventorySlot> MainSlots;

    // 하단 퀵바 (9칸)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 HotbarSize = 9;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TArray<FInventorySlot> HotbarSlots;

    // 현재 선택된 퀵바 인덱스 (0 ~ HotbarSize-1)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 ActiveHotbarIndex = 0;

    // 인벤토리 변경시 브로드캐스트 → 위젯에서 Bind
    UPROPERTY(BlueprintAssignable, Category="Inventory")
    FOnInventoryChanged OnInventoryChanged;

public:
    // --- 기본 인터페이스 ---

    // 인벤토리 전체 초기화
    UFUNCTION(BlueprintCallable, Category="Inventory")
    void InitInventory();

    // 메인+핫바 포함 전체에서 아이템 추가 (간단 버전: 같은 ID 있으면 그 칸에 더함, 아니면 빈칸 찾기)
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool AddItem(FName ItemId, int32 Amount);

    // 지정 슬롯에 아이템 넣기 (UI에서 드래그 드롭 할 때 쓸 수 있음)
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool SetItemInSlot(bool bMainInventory, int32 Index, const FInventorySlot& NewSlot);

    // 아이템 소비 (메인+핫바 포함 전 영역 검색)
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool ConsumeItem(FName ItemId, int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    int32 GetItemCount(FName ItemId) const;

    // 퀵바 선택 변경
    UFUNCTION(BlueprintCallable, Category="Inventory")
    void SetActiveHotbarIndex(int32 NewIndex);

    // 현재 선택된 퀵바 아이템 ID
    UFUNCTION(BlueprintCallable, Category="Inventory")
    FName GetActiveHotbarItemId() const;

    // 망치 장비 여부 (아이템 ID는 나중에 에디터에서 세팅)
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool HasEquippedHammer() const;

    // 망치에 해당하는 아이템 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory")
    FName HammerItemId = TEXT("Hammer_Build");

protected:
    virtual void BeginPlay() override;

private:
    bool AddToArray(TArray<FInventorySlot>& Slots, FName ItemId, int32& InOutRemaining);
    bool ConsumeFromArray(TArray<FInventorySlot>& Slots, FName ItemId, int32& InOutRemaining);
};
