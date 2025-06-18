#include "Items/Fragments/Inv_ItemFragment.h"

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Health Potion Consumed");
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Mana Potion Consumed");
}
