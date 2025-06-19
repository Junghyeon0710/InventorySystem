// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GirdSlots/Inv_EquippedGridSlot.h"

#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"

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
	}
}

FReply UInv_EquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}
