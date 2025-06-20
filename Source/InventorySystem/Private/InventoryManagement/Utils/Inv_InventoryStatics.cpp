// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Utils/Inv_InventoryStatics.h"

#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"

UInv_InventoryComponent* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		 return nullptr;
	}

	UInv_InventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInv_InventoryComponent>();
	return InventoryComponent;
}

EInv_ItemCategory UInv_InventoryStatics::GetItemCategoryFromItemComp(const UInv_ItemComponent* ItemComp)
{
	if (!IsValid(ItemComp))
	{
		return EInv_ItemCategory::None;
	}

	return ItemComp->GetItemManifest().GetItemCategory();
}

void UInv_InventoryStatics::ItemHovered(APlayerController* PC, UInv_InventoryItem* Item)
{
	UInv_InventoryComponent* InventoryComponent = GetInventoryComponent(PC);
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	UInv_InventoryBase* InventoryBase = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryBase))
	{
		return;
	}

	InventoryBase->OnItemHovered(Item);
}

void UInv_InventoryStatics::ItemUnHovered(APlayerController* PC)
{
	UInv_InventoryComponent* InventoryComponent = GetInventoryComponent(PC);
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	UInv_InventoryBase* InventoryBase = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryBase))
	{
		return;
	}

	InventoryBase->OnItemUnhovered();
}

UInv_HoverItem* UInv_InventoryStatics::GetHoverItem(APlayerController* PC)
{
	UInv_InventoryComponent* InventoryComponent = GetInventoryComponent(PC);
	if (!IsValid(InventoryComponent))
	{
		return nullptr;
	}

	UInv_InventoryBase* InventoryBase = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryBase))
	{
		return nullptr;
	}

	return InventoryBase->GetHoverItem();
}

UInv_InventoryBase* UInv_InventoryStatics::GetInventoryWidget(APlayerController* PC)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PC);
	if (!IsValid(IC))
	{
		return nullptr;
	}

	return IC->GetInventoryMenu();
}
