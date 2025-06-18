#include "Items/Manifest//Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"

UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	UInv_InventoryItem* Item = NewObject<UInv_InventoryItem>(NewOuter,UInv_InventoryItem::StaticClass());
	Item->SetItemManifest(*this);

	return Item;
}

void FInv_ItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if ( !IsValid(PickupActorClass) || !IsValid(WorldContextObject))
	{
		return;
	}

	AActor* SpawnActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass,SpawnLocation,SpawnRotation);
	if (!IsValid(SpawnActor))
	{
		return;
	}

	UInv_ItemComponent* ItemComp = SpawnActor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComp);

	ItemComp->InitItemManifest(*this);
}
