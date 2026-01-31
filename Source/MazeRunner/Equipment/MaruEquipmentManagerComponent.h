// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/MRAbilitySet.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "MaruEquipmentManagerComponent.generated.h"


class UActorComponent;
class UMRAbilitySystemComponent;
class UMaruEquipmentDefinition;
class UMaruEquipmentInstance;
class UMaruEquipmentManagerComponent;
class UObject;
struct FFrame;
struct FMaruEquipmentList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FMaruAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FMaruAppliedEquipmentEntry()
	{}

	FString GetDebugString() const;

private:
	friend FMaruEquipmentList;
	friend UMaruEquipmentManagerComponent;

	// The equipment class that got equipped
	UPROPERTY()
	TSubclassOf<UMaruEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UMaruEquipmentInstance> Instance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FMaruAbilitySet_GrantedHandles GrantedHandles;
};

/** List of applied equipment */
USTRUCT(BlueprintType)
struct FMaruEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FMaruEquipmentList()
		: OwnerComponent(nullptr)
	{
	}

	FMaruEquipmentList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FMaruAppliedEquipmentEntry, FMaruEquipmentList>(Entries, DeltaParms, *this);
	}

	UMaruEquipmentInstance* AddEntry(TSubclassOf<UMaruEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(UMaruEquipmentInstance* Instance);

private:
	UMRAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UMaruEquipmentManagerComponent;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FMaruAppliedEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

//
template<>
struct TStructOpsTypeTraits<FMaruEquipmentList> : public TStructOpsTypeTraitsBase2<FMaruEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};


/**
 * Manages equipment applied to a pawn
 * 通常被挂载到Character类上
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UMaruEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	 UMaruEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//在LyraQuickBarComponent::EquipItemInSlot中被调用
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	 UMaruEquipmentInstance* EquipItem(TSubclassOf<UMaruEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	 void UnequipItem(UMaruEquipmentInstance* ItemInstance);

	//~UObject interface
	 virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	//~End of UObject interface

	//~UActorComponent interface
	//virtual void EndPlay() override;
	 virtual void InitializeComponent() override;
	 virtual void UninitializeComponent() override;
	 virtual void ReadyForReplication() override;
	//~End of UActorComponent interface

	/** Returns the first equipped instance of a given type, or nullptr if none are found */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	 UMaruEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UMaruEquipmentInstance> InstanceType);

 	/** Returns all equipped instances of a given type, or an empty array if none are found */
 	UFUNCTION(BlueprintCallable, BlueprintPure)
	 TArray<UMaruEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UMaruEquipmentInstance> InstanceType) const;

	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return (T*)GetFirstInstanceOfType(T::StaticClass());
	}

private:
	UPROPERTY(Replicated)
	FMaruEquipmentList EquipmentList;
};

