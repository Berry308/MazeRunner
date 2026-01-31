// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"

#include "MaruEquipmentDefinition.generated.h"

class AActor;
class UMRAbilitySet;
class UMaruEquipmentInstance;

USTRUCT()
struct FMaruEquipmentActorToSpawn
{
	GENERATED_BODY()

	FMaruEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UMaruEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 * EquipmentInstance;AbilitySetsToGrant;FMaruEquipmentActorToSpawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UMaruEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UMaruEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Class to spawn
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UMaruEquipmentInstance> InstanceType;

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UMRAbilitySet>> AbilitySetsToGrant;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FMaruEquipmentActorToSpawn> ActorsToSpawn;
};
