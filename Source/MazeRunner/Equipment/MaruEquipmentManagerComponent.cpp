// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruEquipmentManagerComponent.h"

#include "AbilitySystem/MRAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MazeRunnerLogChannels.h"
#include "Engine/ActorChannel.h"
#include "MaruEquipmentDefinition.h"
#include "MaruEquipmentInstance.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruEquipmentManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//////////////////////////////////////////////////////////////////////
// FMaruAppliedEquipmentEntry

FString FMaruAppliedEquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}

//////////////////////////////////////////////////////////////////////
// FMaruEquipmentList

void FMaruEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
 	for (int32 Index : RemovedIndices)
 	{
 		const FMaruAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnUnequipped();
		}
 	}
}

//这个不知道在什么时候调用呢
void FMaruEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FMaruAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnEquipped();
		}
	}
}

void FMaruEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
 	//for (int32 Index : ChangedIndices)
 	//{
 	//	const FGameplayTagStack& Stack = Stacks[Index];
 	//	TagToCountMap[Stack.Tag] = Stack.StackCount;
 	//}
}

//通过UAbilitySystemGlobals获取OwnerComponent的Owner的ASC
UMRAbilitySystemComponent* FMaruEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	return Cast<UMRAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

//将EquipmentDefinition的AbilitySets全部GiveToAbilitySystem，生成EquipmentActor
UMaruEquipmentInstance* FMaruEquipmentList::AddEntry(TSubclassOf<UMaruEquipmentDefinition> EquipmentDefinition)
{
	UMaruEquipmentInstance* Result = nullptr;

	check(EquipmentDefinition != nullptr);
 	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());
	
	const UMaruEquipmentDefinition* EquipmentCDO = GetDefault<UMaruEquipmentDefinition>(EquipmentDefinition);

	TSubclassOf<UMaruEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UMaruEquipmentInstance::StaticClass();
	}
	
	FMaruAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<UMaruEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);  //@TODO: Using the actor instead of component as the outer due to UE-127172
	Result = NewEntry.Instance;

	if (UMRAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const TObjectPtr<const UMRAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
		}
	}
	else
	{
		//@TODO: Warning logging?
		UE_LOG(LogMR, Warning, TEXT("%s has no AbilitySystemComponent"), *OwnerComponent->GetOwner()->GetName());
	}

	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);

	MarkItemDirty(NewEntry);

	return Result;
}

void FMaruEquipmentList::RemoveEntry(UMaruEquipmentInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FMaruAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (UMRAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}

			Instance->DestroyEquipmentActors();
			

			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// UMaruEquipmentManagerComponent

UMaruEquipmentManagerComponent::UMaruEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UMaruEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

//调用EquipmentList.AddEntry，调用EquipmentInstance->OnEquipped,并选择性将其添加到复制子物件中
UMaruEquipmentInstance* UMaruEquipmentManagerComponent::EquipItem(TSubclassOf<UMaruEquipmentDefinition> EquipmentClass)
{
	UMaruEquipmentInstance* Result = nullptr;
	if (EquipmentClass != nullptr)
	{
		Result = EquipmentList.AddEntry(EquipmentClass);
		if (Result != nullptr)
		{
			Result->OnEquipped();

			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(Result);
			}
		}
	}
	return Result;
}

void UMaruEquipmentManagerComponent::UnequipItem(UMaruEquipmentInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}

		ItemInstance->OnUnequipped();
		EquipmentList.RemoveEntry(ItemInstance);
	}
}

bool UMaruEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FMaruAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		UMaruEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UMaruEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UMaruEquipmentManagerComponent::UninitializeComponent()
{
	TArray<UMaruEquipmentInstance*> AllEquipmentInstances;

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	
	for (const FMaruAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		AllEquipmentInstances.Add(Entry.Instance);
	}

	for (UMaruEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}

void UMaruEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing LyraEquipmentInstances，这个复制子物件不知道会不会需要哪里开启
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FMaruAppliedEquipmentEntry& Entry : EquipmentList.Entries)
		{
			UMaruEquipmentInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

UMaruEquipmentInstance* UMaruEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UMaruEquipmentInstance> InstanceType)
{
	for (FMaruAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UMaruEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

TArray<UMaruEquipmentInstance*> UMaruEquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<UMaruEquipmentInstance> InstanceType) const
{
	TArray<UMaruEquipmentInstance*> Results;
	for (const FMaruAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UMaruEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}


