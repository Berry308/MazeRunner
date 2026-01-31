// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"

#include "MRPlayerStart.generated.h"


class AController;
class UObject;

enum class EMRPlayerStartLocationOccupancy
{
	Empty,
	Partial,//部分的
	Full
};

/**
 * AMRPlayerStart
 * 
 * Base player starts that can be used by a lot of modes.
 */
UCLASS(MinimalAPI, Config = Game)
class AMRPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	 AMRPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	const FGameplayTagContainer& GetGameplayTags() { return StartPointTags; }

	 EMRPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** Did this player start get claimed by a controller already? */
	 bool IsClaimed() const;

	/** If this PlayerStart was not claimed, claim it for ClaimingController */
	 bool TryClaim(AController* OccupyingController);

protected:
	/** Check if this PlayerStart is still claimed */
	 void CheckUnclaimed();

	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** Tags to identify this player start */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;
};

#undef UE_API
