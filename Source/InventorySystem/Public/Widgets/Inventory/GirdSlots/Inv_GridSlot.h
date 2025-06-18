// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

class UInv_ItemPopUp;
class UInv_InventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);

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
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
	void SetTitleIndex(int32 Index) { TitleIndex = Index;};
	int32 GetTitleIndex() const {return TitleIndex;}
	EInv_GridSlotState GetState() const {return GridSlotState;}
	TWeakObjectPtr<UInv_InventoryItem> GetInventoryItem() const {return InventoryItem;}
	void SetInventoryItem(UInv_InventoryItem* Item);
	int32 GetIndex() const {return TitleIndex;}
	void SetIndex(int32 Index) {UpperLeftIndex = Index;}
	int32 GetUpperLeftIndex() const {return UpperLeftIndex;}
	void SetUpperLeftIndex(int32 Index) {UpperLeftIndex = Index;}
	void SetStackCount(int32 Count) {StackCount = Count;}
	int32 GetStackCount() const {return StackCount;}
	bool IsAvailable() const {return bAvailable;}
	void SetAvailable(bool bIsAvailable) {bAvailable = bIsAvailable;}
	void SetItemPopUp(UInv_ItemPopUp* PopUp);
	UInv_ItemPopUp* GetItemPopUp() const {return ItemPopUp.Get();}

	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSlotHovered;
	FGridSlotEvent GridSlotUnhovered;
	
private:
	int32 TitleIndex{INDEX_NONE};
	int32 StackCount{0};
	int32 UpperLeftIndex{INDEX_NONE};
	bool bAvailable{true};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	TWeakObjectPtr<UInv_ItemPopUp> ItemPopUp;
	
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

	UFUNCTION()
	void OnItemPopupDestruct(UUserWidget* Menu);

public:
	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();
};
