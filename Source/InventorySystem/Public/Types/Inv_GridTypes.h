#pragma once

#include "Inv_GridTypes.generated.h"

class UInv_InventoryItem;

UENUM(BlueprintType)
enum class EInv_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FInv_SlotAvailability
{
	GENERATED_BODY()

	FInv_SlotAvailability() {}
	FInv_SlotAvailability(int32 Index, int32 Room, bool bHasItem) : Index(Index), AmountToFill(Room), bItemAtIndex(bHasItem) {}

	int32 Index{INDEX_NONE};          // 슬롯의 인덱스
	int32 AmountToFill{0};            // 이 슬롯에 얼마나 들어갈 수 있는지
	bool bItemAtIndex{false};         // 이 슬롯에 이미 아이템이 있는지 여부
};

USTRUCT()
struct FInv_SlotAvailabilityResult
{
	GENERATED_BODY()

	FInv_SlotAvailabilityResult() {}
	
	TWeakObjectPtr<UInv_InventoryItem> Item;     // 검사 대상 아이템
	int32 TotalRoomToFill{0};                    // 총 채운 수량
	int32 Remainder{0};                          // 공간 부족시 남은 아이템 수
	bool bStackable{false};                      // 스택 가능한 아이템인지 여부
	TArray<FInv_SlotAvailability> SlotAvailabilities;  // 가능한 슬롯들 정보 리스트
};

UENUM(BlueprintType)
enum class EInv_TileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};

USTRUCT(BlueprintType)
struct FInv_TileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	FIntPoint TileCoordinats{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	int32 TileIndex{INDEX_NONE};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory")
	EInv_TileQuadrant TileQuadrant{EInv_TileQuadrant::None};
};

inline bool operator==(const FInv_TileParameters& A, const FInv_TileParameters& B)
{
	return A.TileCoordinats == B.TileCoordinats && A.TileIndex == B.TileIndex && A.TileQuadrant == B.TileQuadrant;
}

USTRUCT()
struct FInv_SpaceQueryResult
{
	GENERATED_BODY()
	
	// 쿼리한 공간에 아이템이 없으면 true, 공간이 비어있음을 의미
	bool bHasSpace{false};

	// 스왑 가능한 아이템이 있으면 이 포인터가 유효함 (없으면 nullptr)
	TWeakObjectPtr<UInv_InventoryItem> ValidItem = nullptr;

	// 유효한 아이템이 있을 경우, 그 아이템의 좌측 상단 인덱스 (없으면 INDEX_NONE)
	int32 UpperLeftIndex{INDEX_NONE};
};