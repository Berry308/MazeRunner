// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruInventoryItemDefinition.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruInventoryItemDefinition)

//////////////////////////////////////////////////////////////////////
// UMaruInventoryItemDefinition

UMaruInventoryItemDefinition::UMaruInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UMaruInventoryItemFragment* UMaruInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UMaruInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UMaruInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// UMaruInventoryItemDefinition

const UMaruInventoryItemFragment* ULyraInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UMaruInventoryItemDefinition> ItemDef, TSubclassOf<UMaruInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UMaruInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

