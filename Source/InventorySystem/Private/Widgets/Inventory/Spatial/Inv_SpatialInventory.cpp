﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "InventorySystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetSwitcher.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Widgets/ItemDescription/UInv_ItemDescription.h"
#include "Blueprint/WidgetTree.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Widgets/Inventory/GirdSlots/Inv_EquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Equippables->OnClicked.AddDynamic(this,&ThisClass::UInv_SpatialInventory::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this,&ThisClass::UInv_SpatialInventory::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this,&ThisClass::UInv_SpatialInventory::ShowCraftables);

	Grid_Equippables->SetOwningCanvas(CanvasPanel);
	Grid_Consumables->SetOwningCanvas(CanvasPanel);
	Grid_Craftables->SetOwningCanvas(CanvasPanel);
	

	ShowEquippables();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInv_EquippedGridSlot* EquippedGridSlot = Cast<UInv_EquippedGridSlot>(Widget);
		if (IsValid(EquippedGridSlot))
		{
			EquippedGridSlots.Add(EquippedGridSlot);
			EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &ThisClass::EquippedGridSLotClicked);
		}
	});
}

void UInv_SpatialInventory::EquippedGridSLotClicked(UInv_EquippedGridSlot* EquippedGridSlot,const FGameplayTag& EquipmentTypeTag)
{
	if (!CanEquipHoverItem(EquippedGridSlot, EquipmentTypeTag))
	{
		return;
	}
	UInv_HoverItem* HoverItem = GetHoverItem();
	const float TileSize = UInv_InventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	UInv_EquippedSlottedItem* EquippedSlottedItem = EquippedGridSlot->OnItemEquipped(
		HoverItem->GetInventoryItem(),
		EquipmentTypeTag,
		TileSize
	);
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent))

	InventoryComponent->Server_EquipSlotClicked(HoverItem->GetInventoryItem(),nullptr);

	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(HoverItem->GetInventoryItem());
	}
	
	Grid_Equippables->ClearHoverItem();
	
}

void UInv_SpatialInventory::EquippedSlottedItemClicked(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	// 1. 아이템 설명이 표시되고 있었다면 제거합니다.
	UInv_InventoryStatics::ItemUnHovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable())
	{
		return;
	}
	// 2. 현재 클릭한 장착 슬롯 아이템이 소속된 EquippedGridSlot을 찾습니다.

	UInv_InventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
	UInv_InventoryItem* ItemToUnequip = EquippedSlottedItem->GetInventoryItem();

	UInv_EquippedGridSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnequip);
	
	// 3. 해당 장착 슬롯의 InventoryItem을 nullptr로 설정하여 슬롯을 비웁니다.
	ClearSlotOfItem(EquippedGridSlot);

	Grid_Equippables->AssignHoverItem(ItemToUnequip);
	
	// 4. 장착 슬롯에서 장착된 아이템 위젯을 제거합니다.
	RemoveEquippedSlottedItem(EquippedSlottedItem);

	// Make Equipped Slotted Item
	MakeEquippedSlottedItem(EquippedSlottedItem, EquippedGridSlot, ItemToEquip);

	// 5. 장착과 해제에 관련된 델리게이트들을 브로드캐스트합니다.
	//    - OnItemEquipped
	//    - OnItemUnequipped
	BroadcastSlotClickedDelegates(ItemToEquip, ItemToUnequip);
	// ※ 해당 델리게이트는 서버 RPC를 통해 호출되며, Multicast RPC로 전체 클라이언트에 반영됩니다.

}

FReply UInv_SpatialInventory::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ActiveGrid->DropItem();
	return FReply::Handled();
}

void UInv_SpatialInventory::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescription))
	{
		return;
	}
	SetItemDescriptionSizeAndPosition(ItemDescription,CanvasPanel);
	
}

void UInv_SpatialInventory::SetItemDescriptionSizeAndPosition(UInv_ItemDescription* Description, UCanvasPanel* Canvas) const
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(Description);
	if (!IsValid(ItemDescriptionCPS))
	{
		return;
	}

	const FVector2D ItemDescriptionSize = Description->GetSize();
	ItemDescriptionCPS->SetSize(ItemDescriptionSize);

	FVector2D ClampedPosition = UInv_WidgetUtils::GetClampedWidgetPosition(
		UInv_WidgetUtils::GetWidgetSize(Canvas),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer())
		);

	ItemDescriptionCPS->SetPosition(ClampedPosition);
}

bool UInv_SpatialInventory::CanEquipHoverItem(UInv_EquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag) const
{
	if ( !IsValid(EquippedGridSlot) || EquippedGridSlot->GetInventoryItem().IsValid())
	{
		return false;
	}

	UInv_HoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem))
	{
		return false;
	}

	UInv_InventoryItem* HeldItem = HoverItem->GetInventoryItem();

	return HasHoverItem() && IsValid(HeldItem) &&
		!HoverItem->IsStackable() &&
			HeldItem->GetItemManifest().GetItemCategory() == EInv_ItemCategory::Equippable &&
				HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);
}

UInv_EquippedGridSlot* UInv_SpatialInventory::FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem) const
{
	auto* FoundEquippedGridSlot = EquippedGridSlots.FindByPredicate([EquippedItem](const UInv_GridSlot* GridSlot)
	{
		return GridSlot->GetInventoryItem() == EquippedItem;
	});
	return FoundEquippedGridSlot ? *FoundEquippedGridSlot : nullptr;
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	switch (UInv_InventoryStatics::GetItemCategoryFromItemComp(ItemComponent))
	{
		case EInv_ItemCategory::Equippable:
			return Grid_Equippables->HasRoomForItem (ItemComponent);
		case EInv_ItemCategory::Consumable:
			return Grid_Consumables->HasRoomForItem (ItemComponent);
		case EInv_ItemCategory::Craftable:
			return Grid_Craftables->HasRoomForItem (ItemComponent);
		default:
			UE_LOG(LogInventory,Error,TEXT("Item Category Not Found"));
			break;
	}
	FInv_SlotAvailabilityResult Result;
	Result.TotalRoomToFill = 1;
	return Result;
}

void UInv_SpatialInventory::OnItemHovered(UInv_InventoryItem* Item)
{
	const auto& Manifest = Item->GetItemManifest();
	UInv_ItemDescription* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);

	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this, &Manifest, DescriptionWidget]()
	{
		GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible);
		Manifest.AssimilateInventoryFragment(DescriptionWidget);
	});

	GetOwningPlayer()->GetWorldTimerManager().SetTimer(DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
}

void UInv_SpatialInventory::OnItemUnhovered()
{
	GetItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
}

bool UInv_SpatialInventory::HasHoverItem() const
{
	if (Grid_Equippables->HasHoverItem())
	{
		return true;
	}
	if (Grid_Consumables->HasHoverItem())
	{
		return true;
	}
	if (Grid_Craftables->HasHoverItem())
	{
		return true;
	}
	return false;
	
}

UInv_HoverItem* UInv_SpatialInventory::GetHoverItem() const
{
	if (!ActiveGrid.IsValid())
	{
		return nullptr;
	}

	return ActiveGrid->GetHoverItem();
}

float UInv_SpatialInventory::GetTileSize() const
{
	return Grid_Equippables->GetTileSize();
}

UInv_ItemDescription* UInv_SpatialInventory::GetItemDescription()
{
	if (!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UInv_ItemDescription>(GetOwningPlayer(),ItemDescriptionClass);
		CanvasPanel->AddChild(ItemDescription);
	}
	return ItemDescription;
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables,Button_Equippables);
}

void UInv_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables,Button_Consumables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables,Button_Craftables);
}

void UInv_SpatialInventory::DisableButton(UButton* Button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);

	Button->SetIsEnabled(false);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* Button)
{
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->HideCursor();
		ActiveGrid->OnHide();
	}
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->ShowCursor();
	}

	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

void UInv_SpatialInventory::ClearSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot)
{
	if (IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
		EquippedGridSlot->SetInventoryItem(nullptr);
	}
}

void UInv_SpatialInventory::RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem))
	{
		return;
	}

	if (EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this, &ThisClass::EquippedSlottedItemClicked))
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	}
	EquippedSlottedItem->RemoveFromParent();
}

void UInv_SpatialInventory::MakeEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem, UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip)
{
	if (!IsValid(EquippedGridSlot))
	{
		return;
	}

	UInv_EquippedSlottedItem* SlottedItem = EquippedGridSlot->OnItemEquipped(
		ItemToEquip,
		EquippedSlottedItem->GetEquipmentTypeTag(),
		UInv_InventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize());

	if (IsValid(SlottedItem))
	{
		SlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this,  &ThisClass::EquippedSlottedItemClicked);
	}
	
	EquippedGridSlot->SetEquippedSlottedItem(SlottedItem);
}

void UInv_SpatialInventory::BroadcastSlotClickedDelegates(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUbEquip) const
{
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));

	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUbEquip);

	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(ItemToEquip);
		InventoryComponent->OnItemUnEquipped.Broadcast(ItemToUbEquip);
	}
}
