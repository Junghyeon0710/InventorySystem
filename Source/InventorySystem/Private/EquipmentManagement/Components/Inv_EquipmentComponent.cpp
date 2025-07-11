﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"

void UInv_EquipmentComponent::SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh)
{
	OwningSkeletalMesh = OwningMesh;
}

void UInv_EquipmentComponent::InitializeOwner(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		OwningPlayerController = PlayerController;
	}

	InitInventoryComponent();
}

void UInv_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	InitPlayerController();
}

void UInv_EquipmentComponent::InitPlayerController()
{
	OwningPlayerController = Cast<APlayerController>(GetOwner());
	if (OwningPlayerController.IsValid())
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn());
		if (IsValid(OwnerCharacter))
		{
			OnPossessedPawnChanged(nullptr,OwnerCharacter);
		}
		else
		{
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
		}
	}
}

void UInv_EquipmentComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn());
	if (IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}
	InitInventoryComponent();
}

void UInv_EquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(OwningPlayerController.Get());
	if (!InventoryComponent.IsValid())
	{
		return;
	}
	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &ThisClass::OnItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::OnItemEquipped);
	}

	if (!InventoryComponent->OnItemUnEquipped.IsAlreadyBound(this, &ThisClass::OnItemUnEquipped))
	{
		InventoryComponent->OnItemUnEquipped.AddDynamic(this, &ThisClass::OnItemUnEquipped);
	}
}

AInv_EquipActor* UInv_EquipmentComponent::SpawnEquippedActor(FInv_EquipmentFragment* EquipmentFragment, const FInv_ItemManifest& Manifest, USkeletalMeshComponent* AttachMesh)
{
	AInv_EquipActor* SpawnedEquipActor = EquipmentFragment->SpawnAttachedActor(AttachMesh);
	SpawnedEquipActor->SetEquipmentType(EquipmentFragment->GetEquipmentType());
	SpawnedEquipActor->SetOwner(GetOwner());
	EquipmentFragment->SetEquippedActor(SpawnedEquipActor);
	return SpawnedEquipActor;
}

AInv_EquipActor* UInv_EquipmentComponent::FindEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	auto FoundActor = EquippedActors.FindByPredicate([&EquipmentTypeTag](const AInv_EquipActor* EquippedActor)
	{
		return EquippedActor->GetEquipmentType().MatchesTagExact(EquipmentTypeTag);
	});

	return FoundActor ? *FoundActor : nullptr;
}

void UInv_EquipmentComponent::RemoveEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	if (AInv_EquipActor* EquippedActor = FindEquippedActor(EquipmentTypeTag))
	{
		EquippedActors.Remove(EquippedActor);
		EquippedActor->Destroy();
	}
}

void UInv_EquipmentComponent::OnItemEquipped(UInv_InventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem))
	{
		return;
	}

	if (!OwningPlayerController->HasAuthority())
	{
		return;
	}

	FInv_ItemManifest& ItemManifest = EquippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment)
	{
		return;
	}

	if (!bIsProxy)
	{
	EquipmentFragment->OnEquip(OwningPlayerController.Get());
		
	}

	if (!OwningSkeletalMesh.IsValid())
	{
		return;
	}
	AInv_EquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment,ItemManifest,OwningSkeletalMesh.Get());

	EquippedActors.Add(SpawnedEquipActor);
	
}

void UInv_EquipmentComponent::OnItemUnEquipped(UInv_InventoryItem* UnequippedItem)
{
	if (!IsValid(UnequippedItem))
	{
		return;
	}

 	if (!OwningPlayerController->HasAuthority())
	{
		return;
	}

	FInv_ItemManifest& ItemManifest = UnequippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment)
	{
		return;
	}

	if (!bIsProxy)
	{
		EquipmentFragment->OnUnequip(OwningPlayerController.Get());
	}
	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());
}

