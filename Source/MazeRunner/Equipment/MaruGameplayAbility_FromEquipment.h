// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/MRGameplayAbility.h"

#include "MaruGameplayAbility_FromEquipment.generated.h"

class UMaruEquipmentInstance;
class UMaruInventoryItemInstance;

/**
 * UMaruGameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 * 提供接口获取ULyraEquipmentInstance和ULyraInventoryItemInstance，以及数据验证函数
 */
UCLASS()
class UMaruGameplayAbility_FromEquipment : public UMRGameplayAbility
{
	GENERATED_BODY()

public:

	UMaruGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Lyra|Ability")
	UMaruEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	UMaruInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

};
