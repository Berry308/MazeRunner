// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameplayAbility_Jump.h"

#include "AbilitySystem/Abilities/MRGameplayAbility.h"
#include "Character/MRCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRGameplayAbility_Jump)

struct FGameplayTagContainer;


UMRGameplayAbility_Jump::UMRGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UMRGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AMRCharacter* LyraCharacter = Cast<AMRCharacter>(ActorInfo->AvatarActor.Get());
	if (!LyraCharacter || !LyraCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UMRGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRGameplayAbility_Jump::CharacterJumpStart()
{
	if (AMRCharacter* MRCharacter = GetMRCharacterFromActorInfo())
	{
		if (MRCharacter->IsLocallyControlled() && !MRCharacter->bPressedJump)
		{
			MRCharacter->UnCrouch();
			MRCharacter->Jump();
		}
	}
}

void UMRGameplayAbility_Jump::CharacterJumpStop()
{
	if (AMRCharacter* MRCharacter = GetMRCharacterFromActorInfo())
	{
		if (MRCharacter->IsLocallyControlled() && MRCharacter->bPressedJump)
		{
			MRCharacter->StopJumping();
		}
	}
}

