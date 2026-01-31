// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruAbilitySystemGlobals.h"

#include "MRGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruAbilitySystemGlobals)

struct FGameplayEffectContext;

UMaruAbilitySystemGlobals::UMaruAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UMaruAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMRGameplayEffectContext();
}

