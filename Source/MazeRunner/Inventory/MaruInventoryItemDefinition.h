// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "MaruInventoryItemDefinition.generated.h"

template <typename T> class TSubclassOf;

class UMaruInventoryItemInstance;
struct FFrame;

//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UMaruInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UMaruInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * UMaruInventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class UMaruInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UMaruInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UMaruInventoryItemFragment>> Fragments;

public:
	const UMaruInventoryItemFragment* FindFragmentByClass(TSubclassOf<UMaruInventoryItemFragment> FragmentClass) const;
};

//@TODO: Make into a subsystem instead?
UCLASS()
class ULyraInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UMaruInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UMaruInventoryItemDefinition> ItemDef, TSubclassOf<UMaruInventoryItemFragment> FragmentClass);
};
