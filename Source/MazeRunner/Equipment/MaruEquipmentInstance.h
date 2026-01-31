// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/World.h"

#include "MaruEquipmentInstance.generated.h"

class AActor;
class APawn;
class UAnimMontage;
class UAnimInstance;
struct FFrame;
struct FMaruEquipmentActorToSpawn;

/**
 * UMaruEquipmentInstance
 *
 * A piece of equipment spawned and applied to a pawn
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class UMaruEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	UMaruEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual UWorld* GetWorld() const override final;
	//~End of UObject interface

	UFUNCTION(BlueprintPure, Category=Equipment)
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	UFUNCTION(BlueprintPure, Category=Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure, Category=Equipment)
	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

	UFUNCTION(BlueprintCallable)
	TSubclassOf<UAnimInstance> GetFirstPersonAnimInstanceClass() const;

	UFUNCTION(BlueprintCallable)
	TSubclassOf<UAnimInstance> GetThirdPersonAnimInstanceClass() const;

	virtual void SpawnEquipmentActors(const TArray<FMaruEquipmentActorToSpawn>& ActorsToSpawn);
	virtual void DestroyEquipmentActors();

	//UMaruEquipmentManagerComponent::EquipItem; FMaruEquipmentList::PostReplicatedAdd
	virtual void OnEquipped();
	virtual void OnUnequipped();

protected:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	//virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	TObjectPtr<UAnimMontage> CharacterEquipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	TObjectPtr<UAnimMontage> CharacterHoldEquipAnim;*/

	/** AnimInstance class to set for the first person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** AnimInstance class to set for the third person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnUnequipped"))
	void K2_OnUnequipped();

private:
	UFUNCTION()
	void OnRep_Instigator();

private:
	UPROPERTY(ReplicatedUsing=OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	//在EquipmentDefinition中也有ActorToSpawn何意味？这里这个是用来记录已经生成的Actors
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AActor>> SpawnedActors;
};
