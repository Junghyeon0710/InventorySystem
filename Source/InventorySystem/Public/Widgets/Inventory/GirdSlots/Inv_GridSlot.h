// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

class UImage;
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

private:
	int32 TitleIndex;

	UPROPERTY(meta  = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;
};
