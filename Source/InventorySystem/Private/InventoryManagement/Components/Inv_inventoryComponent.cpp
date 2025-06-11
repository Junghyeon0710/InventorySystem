// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_inventoryComponent.h"

#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"


UInv_inventoryComponent::UInv_inventoryComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
	
}

void UInv_inventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ConstructInventory();
}

void UInv_inventoryComponent::ConstructInventory()
{
	OwningController = Cast<APlayerController>(GetOwner());
	checkf(OwningController.IsValid() , TEXT("Inventory component should have a Player Controller as Owner."));

	if (!OwningController->IsLocalController())
	{
		return;
	}

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningController.Get(),InventoryMenuClass);
	InventoryMenu->AddToViewport();
}

