﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Inv_Highlightable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInv_Highlightable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYSYSTEM_API IInv_Highlightable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:


	UFUNCTION(BlueprintNativeEvent, Category="Inventory")
	void Highlight();

	UFUNCTION(BlueprintNativeEvent, Category="Inventory")
	void Unhighlight();

	void Test();
};
