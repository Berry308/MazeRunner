// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MRPawnData.generated.h"


class APawn;
class UMRAbilitySet;
class UMRAbilityTagRelationshipMapping;
class UMRCameraMode;
class UMRInputConfig;
class UObject;


/**
 * UMRPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "MazeRunner Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class MAZERUNNER_API UMRPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UMRPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from ALyraPawn or AMRCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MazeRunner|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	TArray<TObjectPtr<UMRAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MazeRunner|Abilities")
	TObjectPtr<UMRAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MazeRunner|Input")
	TObjectPtr<UMRInputConfig> InputConfig;

	//// Default camera mode used by player controlled pawns.
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Camera")
	//TSubclassOf<UMRCameraMode> DefaultCameraMode;
};