﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Inv_InventoryStatics.generated.h"

class UInv_InventoryBase;
class UInv_InventoryItem;
class UInv_InventoryComponent;
class UInv_HoverItem;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInv_InventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="Inventory")
	static UInv_InventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	static EInv_ItemCategory GetItemCategoryFromItemComp(const UInv_ItemComponent* ItemComp);

	template<typename T, typename FuncT>
	static void ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GirdColumns, const FuncT& Func);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	static void ItemHovered(APlayerController* PC, UInv_InventoryItem* Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	static void ItemUnHovered(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	static UInv_HoverItem* GetHoverItem(APlayerController* PC);

	static UInv_InventoryBase* GetInventoryWidget(APlayerController* PC);
};

template<typename T, typename FuncT>
void UInv_InventoryStatics::ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Func)
{
	for (int32 j = 0; j < Range2D.Y; ++j)
	{
		for (int32 i = 0; i < Range2D.X; ++i)
		{
			const FIntPoint Coordinates = UInv_WidgetUtils::GetPositionFromIndex(Index, GridColumns) + FIntPoint(i, j);
			const int32 TileIndex = UInv_WidgetUtils::GetIndexFromPosition(Coordinates, GridColumns);

			if (Array.IsValidIndex(TileIndex))
			{
				Func(Array[TileIndex]);
			}
		}
	}
}
