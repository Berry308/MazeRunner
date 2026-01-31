// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "MRGameData.generated.h"


class UGameplayEffect;
class UObject;

/**
 * UMRGameData
 *
 *	Non-mutable data asset that contains global game data.
 * 主要包含三种GameplayEffect
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "MazeRunner Game Data", ShortTooltip = "Data asset containing global game data."))
class UMRGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	 UMRGameData();

	// Returns the loaded game data.通过LyraAssetManager获取的
	static  const UMRGameData& Get();

public:

	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};

#undef UE_API
