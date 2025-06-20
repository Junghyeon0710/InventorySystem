// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GirdSlots/Inv_EquippedGridSlot.h"

#include "Components/Image.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"

void UInv_EquippedGridSlot::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!IsAvailable())
	{
		return;
	}

	UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem))
	{
		return;
	}

	if (HoverItem->GetItemTag().MatchesTag(EquipmentTypeTag))
	{
		SetOccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInv_EquippedGridSlot::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (!IsAvailable())
	{
		return;
	}

	UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem))
	{
		return;
	}

	if (HoverItem->GetItemTag().MatchesTag(EquipmentTypeTag))
	{
		SetUnoccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UInv_EquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

UInv_EquippedSlottedItem* UInv_EquippedGridSlot::OnItemEquipped(UInv_InventoryItem* Item, const FGameplayTag& EquipmentTag, float TileSize)
{
	// 1. 장비 타입 태그가 적합한지 검사합니다. (예: 망토 슬롯에는 망토만 장착 가능)
	if (!EquipmentTag.MatchesTag(EquipmentTypeTag))
	{
		return nullptr;
	}

	// 2. 슬롯 그리드의 크기를 구합니다.
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment)
	{
		return nullptr;
	}
	const FIntPoint GridDimensions = GridFragment->GetGridSize();
	
	// 3. 그리드 크기를 바탕으로 슬롯 아이템의 그려질 크기(드로우 사이즈)를 계산합니다.
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D DrawSize = GridDimensions * IconTileWidth;
	
	// 4. 새로운 장착 슬롯 아이템 위젯을 생성합니다.
	EquippedSlottedItem = CreateWidget<UInv_EquippedSlottedItem>(GetOwningPlayer(), EquippedSlotClass);
	
	// 5. 새로 생성한 슬롯 아이템에 아이템 데이터를 세팅합니다.
	EquippedSlottedItem->SetInventoryItem(Item);
	
	// 6. 슬롯 아이템에 장비 타입 태그를 세팅합니다.
	EquippedSlottedItem->SetEquipmentTypeTag(EquipmentTag);
	
	// 7. 장착 아이템은 쌓을 수 없으니, 스택 카운트 위젯은 숨깁니다.
	EquippedSlottedItem->UpdateStackCount(0);
	
	// 8. 장착 그리드 슬롯 자신도 현재 장착한 인벤토리 아이템을 추적할 수 있도록 세팅합니다.
	SetInventoryItem(Item);
	
	// 9. 슬롯 아이템에 아이템 이미지를 세팅합니다.

	// 10. 슬롯 아이템 위젯을 장착 그리드 슬롯 위젯에 자식으로 추가합니다.

	// 11. 마지막으로 생성한 장착 슬롯 아이템 위젯을 반환합니다.

}
