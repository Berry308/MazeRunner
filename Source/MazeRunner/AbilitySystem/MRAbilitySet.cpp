// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRAbilitySet.h"

#include "AbilitySystem/Abilities/MRGameplayAbility.h"
#include "MRAbilitySystemComponent.h"
#include "MazeRunnerLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRAbilitySet)

void FMaruAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FMaruAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FMaruAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FMaruAbilitySet_GrantedHandles::TakeFromAbilitySystem(UMRAbilitySystemComponent* LyraASC)
{
	check(LyraASC);

	if (!LyraASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			LyraASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			LyraASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		LyraASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UMRAbilitySet::UMRAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMRAbilitySet::GiveToAbilitySystem(UMRAbilitySystemComponent* MaruASC, FMaruAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(MaruASC);

	if (!MaruASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}
	
	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FMaruAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogMRAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(MaruASC->GetOwner(), SetToGrant.AttributeSet);
		MaruASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FMaruAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogMRAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UMRGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UMRGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		//UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s give GameplayAbility %s to %s"), *this->GetName(), *AbilitySpec.Ability.GetFName().ToString(), *MaruASC->AbilityActorInfo->AvatarActor->GetName());这个Log会崩溃的
		FString AbilityName = AbilitySpec.Ability ? AbilitySpec.Ability->GetFName().ToString() : TEXT("InvalidAbility");
		FString AvatarActorName = TEXT("None");
		if (MaruASC && MaruASC->AbilityActorInfo && MaruASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			AvatarActorName = MaruASC->AbilityActorInfo->AvatarActor->GetName();
		}
		UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s give GameplayAbility %s to %s"),
			*GetNameSafe(this),
			*AbilityName,
			*AvatarActorName);


		const FGameplayAbilitySpecHandle AbilitySpecHandle = MaruASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant the gameplay effects.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FMaruAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogMRAbilitySystem, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = MaruASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, MaruASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}

