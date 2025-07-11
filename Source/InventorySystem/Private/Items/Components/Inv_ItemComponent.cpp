﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/Inv_ItemComponent.h"

#include "Net/UnrealNetwork.h"


UInv_ItemComponent::UInv_ItemComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

	PickupMessage = "E - Pick Up";
	SetIsReplicatedByDefault(true);

}

void UInv_ItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,ItemManifest);
}

void UInv_ItemComponent::InitItemManifest(FInv_ItemManifest CopyOfManifest)
{
	ItemManifest = CopyOfManifest;
}

void UInv_ItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}



