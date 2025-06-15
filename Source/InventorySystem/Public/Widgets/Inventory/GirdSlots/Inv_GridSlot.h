// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

class UImage;

UENUM(BlueprintType)
enum class EInv_GridSlotState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GrayedOut
	
};
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInv_GridSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTitleIndex(int32 Index) { TitleIndex = Index;};
	int32 GetTitleIndex() const {return TitleIndex;}
	EInv_GridSlotState GetState() const {return GridSlotState;}

private:
	int32 TitleIndex;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Unoccupied;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere, Category="Inventory")
	FSlateBrush Brush_GrayedOut;

	EInv_GridSlotState GridSlotState{EInv_GridSlotState::Unoccupied};

public:
	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();
};
