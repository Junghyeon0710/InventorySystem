// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Inv_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class INVENTORYSYSTEM_API AInv_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	void PrimaryInteract();
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TArray<TObjectPtr<UInputMappingContext>> DefaultIMCs;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;
};

