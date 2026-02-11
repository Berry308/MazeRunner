// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruEquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Character/MRCharacter.h"
#include "MaruEquipmentDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
//#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

UMaruEquipmentInstance::UMaruEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UMaruEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void UMaruEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

#if UE_WITH_IRIS
//void UMaruEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
//{
//	using namespace UE::Net;
//
//	// Build descriptors and allocate PropertyReplicationFragments for this object
//	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
//}
#endif // UE_WITH_IRIS

APawn* UMaruEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

//安全地获取当前装备实例所归属的Pawn（角色）对象，且仅当这个Pawn是指定类型时才返回。
APawn* UMaruEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UMaruEquipmentInstance::SpawnEquipmentActors(const TArray<FMaruEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		USceneComponent* AttachTargetFirstPerson = OwningPawn->GetRootComponent();
		if (AMRCharacter* Char = Cast<AMRCharacter>(OwningPawn))
		{
			AttachTargetFirstPerson = Char->GetFirstPersonMesh();
		}

		for (const FMaruEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			//const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget,false);

			AActor* ThirdPersonNewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			ThirdPersonNewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			if (USkeletalMeshComponent* MeshComp = ThirdPersonNewActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				MeshComp->SetOwnerNoSee(true);
			}
			ThirdPersonNewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			ThirdPersonNewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			if (AttachTargetFirstPerson)
			{
				AActor* FirstPersonNewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
				FirstPersonNewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
				if (USkeletalMeshComponent* MeshComp = FirstPersonNewActor->FindComponentByClass<USkeletalMeshComponent>())
				{
					MeshComp->SetOnlyOwnerSee(true);
				}
				FirstPersonNewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
				FirstPersonNewActor->AttachToComponent(AttachTargetFirstPerson, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

				SpawnedActors.Add(FirstPersonNewActor);
			}

			SpawnedActors.Add(ThirdPersonNewActor);
		}
	}
}

void UMaruEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void UMaruEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UMaruEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UMaruEquipmentInstance::OnRep_Instigator()
{
}

TSubclassOf<UAnimInstance> UMaruEquipmentInstance::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

TSubclassOf<UAnimInstance> UMaruEquipmentInstance::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}

