// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/MRGameplayAbility.h"
//#include "Interaction/InteractionOption.h"

#include "MRGameplayAbility_Interact.generated.h"

//class UIndicatorDescriptor;
class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitInputRelease;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UMRGameplayAbility_Interact
 *
 * Gameplay ability used for character interacting
 * 在CanActivateAbility中进行是否能交互的判断
 * 后续可能会单独设计一个组件用来每帧实时检测并存储可交互物体(使用双端队列)，并显示UI
 * 然后交互能力是通过这个组件获取当前选择的可交互物体，并激活可交互物体的自定义逻辑
 */
UCLASS(Abstract)
class UMRGameplayAbility_Interact : public UMRGameplayAbility
{
	GENERATED_BODY()

public:

	UMRGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	//追踪多个可交互目标
	//UFUNCTION(BlueprintCallable)
	//TArray<FHitResult> PerformTrace(AMRCharacter* InSourceActor) const; 

	//UFUNCTION(BlueprintCallable)
	//bool CanInteractWith(AActor* SourceActor,AActor* TargetToInteract) const;

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

	UFUNCTION()
	void OnInputPressed(float TimeWaited);

	UFUNCTION()
	void OnInputHeld(float TimeWaited);

	UFUNCTION()
	void OnInputReleased(float TimeWaited);

	//UPROPERTY(EditDefaultsOnly)
	//float InteractSphereRadius = 10.f;

protected:
	UPROPERTY()
	UAbilityTask_WaitInputPress* PressTask;

	UPROPERTY()
	UAbilityTask_WaitInputRelease* ReleaseTask;

private:
	AActor* CurrentTargetToInteract;

	/*TArray<AActor*> TargetCanBeInteracted;
	int8 CurrentTargetToInteractIndex;
	bool bIsChosen=false;*/
};
