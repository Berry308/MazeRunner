// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"

#include "MRCharacterMovementComponent.generated.h"


class UObject;
struct FFrame;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

/**
 * FMRCharacterGroundInfo
 *
 *	Information about the ground under the character.  It only gets updated as needed.
 */
USTRUCT(BlueprintType)
struct FMRCharacterGroundInfo
{
	GENERATED_BODY()

	FMRCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * UMRCharacterMovementComponent
 *
 *	The base character movement component class used by this project.
 */
UCLASS(MinimalAPI, Config = Game)
class UMRCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	 UMRCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	 //在Lyra中会选择性使用复制加速
	 virtual void SimulateMovement(float DeltaTime) override;

	 // Same as UCharacterMovementComponent's implementation but without the crouch check
	 virtual bool CanAttemptJump() const override;

	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "MazeRunner|CharacterMovement")
	 const FMRCharacterGroundInfo& GetGroundInfo();

	 void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	 virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	 virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	 virtual void InitializeComponent() override;

protected:

	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FMRCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};

