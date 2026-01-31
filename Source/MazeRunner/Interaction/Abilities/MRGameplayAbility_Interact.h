// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/MRGameplayAbility.h"
//#include "Interaction/InteractionOption.h"

#include "MRGameplayAbility_Interact.generated.h"

//class UIndicatorDescriptor;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UMRGameplayAbility_Interact
 *
 * Gameplay ability used for character interacting
 */
UCLASS(Abstract)
class UMRGameplayAbility_Interact : public UMRGameplayAbility
{
	GENERATED_BODY()

public:

	UMRGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//追踪多个可交互目标
	UFUNCTION(BlueprintCallable)
	TArray<FHitResult> PerformTrace(AMRCharacter* InSourceActor);

	UFUNCTION(BlueprintCallable)
	bool CanInteractWith(AActor* SourceActor,AActor* TargetToInteract);

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction(AActor* TargetToInteract);

	UPROPERTY(EditDefaultsOnly)
	float InteractSphereRadius = 10.f;
};
