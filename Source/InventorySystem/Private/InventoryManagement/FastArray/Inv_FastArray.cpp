﻿#include "InventoryManagement/FastArray/Inv_FastArray.h"

TArray<UInv_InventoryItem*> FInv_InventoryFastArray::GetItems() const
{
}

void FInv_InventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FInv_InventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_InventoryComponent* ItemComponent)
{
}

UInv_InventoryItem* FInv_InventoryFastArray::AddEntry(UInv_InventoryItem* Item)
{
}

void FInv_InventoryFastArray::RemoveEntry(UInv_InventoryItem* Item)
{
}
