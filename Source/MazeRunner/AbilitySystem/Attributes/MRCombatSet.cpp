// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRCombatSet.h"

#include "AbilitySystem/Attributes/MRAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRCombatSet)

class FLifetimeProperty;


UMRCombatSet::UMRCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

void UMRCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMRCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMRCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UMRCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMRCombatSet, BaseDamage, OldValue);
}

void UMRCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMRCombatSet, BaseHeal, OldValue);
}

