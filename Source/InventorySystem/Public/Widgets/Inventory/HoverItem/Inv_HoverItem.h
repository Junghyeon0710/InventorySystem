﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Inv_HoverItem.generated.h"

class UInv_InventoryItem;
/**
 * 
 */
class UImage;
class UTextBlock;
UCLASS()
class INVENTORYSYSTEM_API UInv_HoverItem : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetImageBrush(const FSlateBrush& Brush) const;
	void UpdateStackCount(const int32 Count);

	FGameplayTag GetItemTag() const;
	int32 GetStackCount() const {return StackCount;}
	bool IsStackable() const {return bIsStackable;}
	void SetIsStackable(bool bStacks);
	int32 GetPreviousGridIndex() const {return PreviousGridIndex;}
	void SetPreviousGridIndex(int32 Index) {PreviousGridIndex = Index;}
	FIntPoint GetGridDimensions() const {return GridDimensions;}
	void SetGridDimensions(const FIntPoint& Dimensions) {GridDimensions = Dimensions;}
	UInv_InventoryItem* GetInventoryItem() const;
	void SetInventoryItem(UInv_InventoryItem* Item);

private:

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	int32 PreviousGridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{false};
	int32 StackCount{1};

};
