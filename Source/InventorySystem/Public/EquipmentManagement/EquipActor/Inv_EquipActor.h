﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Inv_EquipActor.generated.h"

UCLASS()
class INVENTORYSYSTEM_API AInv_EquipActor : public AActor
{
	GENERATED_BODY()

public:
	AInv_EquipActor();
	FGameplayTag GetEquipmentType() const {return EquipmentType;}
	void SetEquipmentType(FGameplayTag Type) { EquipmentType = Type; };

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;
};
