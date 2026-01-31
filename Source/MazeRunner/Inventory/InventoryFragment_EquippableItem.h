// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/MaruInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "InventoryFragment_EquippableItem.generated.h"

class UMaruEquipmentDefinition;
class UObject;

UCLASS()
class UInventoryFragment_EquippableItem : public UMaruInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Lyra)
	TSubclassOf<UMaruEquipmentDefinition> EquipmentDefinition;
};
