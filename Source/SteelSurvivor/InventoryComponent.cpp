// InventoryComponent.cpp

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MainCols = 5;
    MainRows = 2;
    HotbarSize = 9;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InitInventory();
}

void UInventoryComponent::InitInventory()
{
    MainSlots.SetNum(MainCols * MainRows);
    HotbarSlots.SetNum(HotbarSize);

    for (FInventorySlot& Slot : MainSlots)
    {
        Slot.ItemId = NAME_None;
        Slot.Count = 0;
    }
    for (FInventorySlot& Slot : HotbarSlots)
    {
        Slot.ItemId = NAME_None;
        Slot.Count = 0;
    }

    ActiveHotbarIndex = 0;

    OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::AddItem(FName ItemId, int32 Amount)
{
    if (ItemId.IsNone() || Amount <= 0) return false;

    int32 Remaining = Amount;

    // 1) 메인 인벤 토합
    if (!AddToArray(MainSlots, ItemId, Remaining))
        return false;

    // 2) 그래도 남으면 퀵바에 시도
    if (Remaining > 0)
    {
        if (!AddToArray(HotbarSlots, ItemId, Remaining))
        {
            // 메인에는 들어갔는데, 퀵바에 못 들어간 건 괜찮음
        }
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::AddToArray(TArray<FInventorySlot>& Slots, FName ItemId, int32& InOutRemaining)
{
    if (InOutRemaining <= 0)
	{
		return true;
	}
	
    // 같은 ID 있는 슬롯에 먼저 채우기 (스택 제한은 나중에 추가)
    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemId == ItemId && Slot.Count > 0)
        {
            Slot.Count += InOutRemaining;
            InOutRemaining = 0;
            return true;
        }
    }

    // 빈 슬롯 찾아 넣기
    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty())
        {
            Slot.ItemId = ItemId;
            Slot.Count = InOutRemaining;
            InOutRemaining = 0;
            return true;
        }
    }

    // 전혀 못 넣었으면 false
    return InOutRemaining <= 0;
}

bool UInventoryComponent::SetItemInSlot(bool bMainInventory, int32 Index, const FInventorySlot& NewSlot)
{
    TArray<FInventorySlot>& Slots = bMainInventory ? MainSlots : HotbarSlots;
    if (!Slots.IsValidIndex(Index)) return false;

    Slots[Index] = NewSlot;
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::ConsumeItem(FName ItemId, int32 Amount)
{
    if (ItemId.IsNone() || Amount <= 0) return false;

    int32 Remaining = Amount;

    // 메인 인벤에서 먼저 소비
    ConsumeFromArray(MainSlots, ItemId, Remaining);
    // 남으면 퀵바에서도 소비
    ConsumeFromArray(HotbarSlots, ItemId, Remaining);

    const bool bSuccess = (Remaining <= 0);
    if (bSuccess)
    {
        OnInventoryChanged.Broadcast();
    }
    return bSuccess;
}

bool UInventoryComponent::ConsumeFromArray(TArray<FInventorySlot>& Slots, FName ItemId, int32& InOutRemaining)
{
    if (InOutRemaining <= 0) return true;

    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemId == ItemId && Slot.Count > 0)
        {
            const int32 Use = FMath::Min(Slot.Count, InOutRemaining);
            Slot.Count -= Use;
            InOutRemaining -= Use;

            if (Slot.Count <= 0)
            {
                Slot.ItemId = NAME_None;
                Slot.Count = 0;
            }

            if (InOutRemaining <= 0)
                return true;
        }
    }
    return InOutRemaining <= 0;
}

int32 UInventoryComponent::GetItemCount(FName ItemId) const
{
    if (ItemId.IsNone()) return 0;
    int32 Total = 0;

    for (const FInventorySlot& Slot : MainSlots)
    {
        if (Slot.ItemId == ItemId)
            Total += Slot.Count;
    }
    for (const FInventorySlot& Slot : HotbarSlots)
    {
        if (Slot.ItemId == ItemId)
            Total += Slot.Count;
    }

    return Total;
}

void UInventoryComponent::SetActiveHotbarIndex(int32 NewIndex)
{
    if (HotbarSlots.Num() == 0) return;

    ActiveHotbarIndex = FMath::Clamp(NewIndex, 0, HotbarSlots.Num() - 1);
    OnInventoryChanged.Broadcast();
}

FName UInventoryComponent::GetActiveHotbarItemId() const
{
    if (!HotbarSlots.IsValidIndex(ActiveHotbarIndex))
        return NAME_None;

    return HotbarSlots[ActiveHotbarIndex].ItemId;
}

bool UInventoryComponent::HasEquippedHammer() const
{
    if (HammerItemId.IsNone()) return false;
    return GetActiveHotbarItemId() == HammerItemId;
}
