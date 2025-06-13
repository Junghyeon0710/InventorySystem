#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Inv_FastArray.generated.h"

class UInv_InventoryComponent;
class UInv_InventoryItem;
class UInv_ItemComponent;

/** 인벤토리 안의 단일 항목 */
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInv_InventoryEntry() {}

private:
	friend struct FInv_InventoryFastArray;
	friend UInv_InventoryComponent;

	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
};

/** 인벤토리 아이템들의 리스트 */
USTRUCT(BlueprintType)
struct FInv_InventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	FInv_InventoryFastArray() : OwnerComponent(nullptr) {}
	FInv_InventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	TArray<UInv_InventoryItem*> GetItems() const;

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	//void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FInv_InventoryEntry,FInv_InventoryFastArray>(Entries,DeltaParms,*this);
	}

	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_InventoryItem* Item);
	void RemoveEntry(UInv_InventoryItem* Item);

private:
	friend UInv_InventoryComponent;

	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : public  TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	enum { WithNetDeltaSerializer = true };// 이 타입은 델타 직렬화를 지원한다};
};