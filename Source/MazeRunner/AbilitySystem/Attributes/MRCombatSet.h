// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "MRAttributeSet.h"

#include "MRCombatSet.generated.h"

class UObject;
struct FFrame;


/**
 * UMRCombatSet
 *
 *  Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 *  用于接收角色、武器、Buff等的动态修改，然后在DamageExecution中被调用作为基础数值
 */
UCLASS(BlueprintType)
class UMRCombatSet : public UMRAttributeSet
{
	GENERATED_BODY()

public:

	UMRCombatSet();

	ATTRIBUTE_ACCESSORS(UMRCombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UMRCombatSet, BaseHeal);

protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "MazeRunner|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// The base amount of healing to apply in the heal execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "MazeRunner|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};
