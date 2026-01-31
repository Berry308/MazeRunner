// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRAttributeSet.h"

#include "AbilitySystem/MRAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRAttributeSet)

class UWorld;


UMRAttributeSet::UMRAttributeSet()
{
}

UWorld* UMRAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UMRAbilitySystemComponent* UMRAttributeSet::GetMRAbilitySystemComponent() const
{
	return Cast<UMRAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

