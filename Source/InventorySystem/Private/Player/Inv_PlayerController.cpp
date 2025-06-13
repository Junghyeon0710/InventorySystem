﻿// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/Inv_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HUD/Inv_HUDWidget.h"
#include "Interaction/Inv_Highlightable.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Kismet/GameplayStatics.h"

AInv_PlayerController::AInv_PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceLength = 500.0f;
	ItemTraceChannel = ECC_GameTraceChannel1;
}

void AInv_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TraceForItem();
}

void AInv_PlayerController::ToggleInventory()
{
	if (!InventoryComponent.IsValid())
	{
		return;
	}

	InventoryComponent->ToggleInventoryMenu();
}


void AInv_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(IsValid(Subsystem))
	{
		for(UInputMappingContext* CurrentContext : DefaultIMCs)
		{
			Subsystem->AddMappingContext(CurrentContext,0);
		}
	}

	InventoryComponent = FindComponentByClass<UInv_InventoryComponent>();
	CreateHUDWidget();
}

void AInv_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(PrimaryInteractAction,ETriggerEvent::Started,this,&ThisClass::PrimaryInteract);
	EnhancedInputComponent->BindAction(ToggleInventoryAction,ETriggerEvent::Started,this,&ThisClass::ToggleInventory);
	
}

void AInv_PlayerController::PrimaryInteract()
{
	if (!ThisActor.IsValid())
	{
		return;
	}

	UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
	if (!IsValid(ItemComponent) || !InventoryComponent.IsValid())
	{
		return;
	}

	InventoryComponent->TryAddItem(ItemComponent);

}

void AInv_PlayerController::CreateHUDWidget()
{
	 if(!IsLocalController())
	 {
		  return;
	 }

	HUDWidget = CreateWidget<UInv_HUDWidget>(this,HUDWidgetClass);
	if(IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}

void AInv_PlayerController::TraceForItem()
{
	if (!IsValid(GEngine) || !GEngine->GameViewport)
	{
		return;
	}
	
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2d ViewportCenter = ViewportSize * 0.5f;

	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter,TraceStart,Forward))
	{
		return;
	}

	const FVector TraceEnd = TraceStart + Forward * TraceLength;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	if (!ThisActor.IsValid())
	{
		if (IsValid(HUDWidget))
		{
			HUDWidget->HidePickupMessage();
		}
	}

	if (ThisActor == LastActor)
	{
		return;
	}

	if (ThisActor.IsValid())
	{

		if (UActorComponent* Highlightable = ThisActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable))
		{
			IInv_Highlightable::Execute_Highlight(Highlightable);
		}	
		
		UInv_ItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInv_ItemComponent>();
		if (!IsValid(ItemComponent))
		{
			return;	
		}
		if (IsValid(HUDWidget))
		{
			HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage());
		}
	}

	if (LastActor.IsValid())
	{
		if (UActorComponent* Highlightable = LastActor->FindComponentByInterface(UInv_Highlightable::StaticClass()); IsValid(Highlightable))
		{
			IInv_Highlightable::Execute_Unhighlight(Highlightable);
		}	
	}
}


