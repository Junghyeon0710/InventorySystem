﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_ItemPopUp.generated.h"

class USizeBox;
class UTextBlock;
class UButton;
class USlider;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopUpMenuSplit, int32, SplitAmount, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuDrop, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuConsume, int32, Index);


UCLASS()
class INVENTORYSYSTEM_API UInv_ItemPopUp : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;

	FPopUpMenuSplit OnSplit;
	FPopUpMenuDrop OnDrop;
	FPopUpMenuConsume OnConsume;
	
	int32 GetSplitAmount() const;
private:

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UButton> Button_Split;
	
	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UButton> Button_Drop;
	
	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UTextBlock> Slider_SplitAmount;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	int32 GrindIndex{INDEX_NONE};

	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void DropButtonClicked();
	
	UFUNCTION()
	void ConsumeButtonClicked();

	UFUNCTION()
	void SliderValueChanged(float Value);
};
