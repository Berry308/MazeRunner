// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruGameplayAbility_FromEquipment.h"
#include "MaruEquipmentInstance.h"
#include "Inventory/MaruInventoryItemInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruGameplayAbility_FromEquipment)

UMaruGameplayAbility_FromEquipment::UMaruGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UMaruEquipmentInstance* UMaruGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UMaruEquipmentInstance>(Spec->SourceObject.Get());//不知道要不要这个ILyraAbilitySourceInterface？好像没什么关系
	}

	return nullptr;
}

UMaruInventoryItemInstance* UMaruGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	if (UMaruEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UMaruInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}


#if WITH_EDITOR
EDataValidationResult UMaruGameplayAbility_FromEquipment::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	{
		Context.AddError(NSLOCTEXT("Lyra", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif
