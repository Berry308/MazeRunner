// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "Interaction/IInteractableTarget.h"
//#include "Interaction/InteractionStatics.h"
//#include "Interaction/Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "NativeGameplayTags.h"
#include "Player/MRPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Character/MRCharacter.h"
#include "MazeRunnerLogChannels.h"
//#include "UI/IndicatorSystem/IndicatorDescriptor.h"
//#include "UI/IndicatorSystem/LyraIndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRGameplayAbility_Interact)

class UUserWidget;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");
UE_DEFINE_GAMEPLAY_TAG(TAG_INTERACTION_DURATION_MESSAGE, "Ability.Interaction.Duration.Message");

UMRGameplayAbility_Interact::UMRGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EMRAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}


void UMRGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s is activating MRGameplayAbility_Interact"), *ActorInfo->OwnerActor->GetName());
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		//判断是否能与某物体交互
		AMRCharacter* Character = Cast<AMRCharacter>(ActorInfo->AvatarActor);
		if (Character)
		{
			//判断是否指向了物体，做一个扫描检测并返回FHitResult
			TArray<FHitResult> Hits;	
			Hits = PerformTrace(Character);
			if (!Hits.IsEmpty())
			{
				//判断每个检测到的目标是否能被交互
				for (FHitResult hit : Hits)
				{
					AActor* HitTarget = hit.GetActor();
					if (CanInteractWith(ActorInfo->AvatarActor.Get(), HitTarget))
					{
						TriggerInteraction(HitTarget);
					}
					else
					{
						UE_LOG(LogMR, Warning, TEXT("%s try interact with %s false"), *ActorInfo->OwnerActor->GetName(),*HitTarget->GetName());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogMR, Warning, TEXT("%s UMRGameplayAbility_Interact Activate false1"), *ActorInfo->OwnerActor->GetName());
		}
	}
	else
	{
		UE_LOG(LogMR, Warning, TEXT("%s UMRGameplayAbility_Interact Activate false2"), *ActorInfo->OwnerActor->GetName());
	}

	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,false,false);
}

TArray<FHitResult> UMRGameplayAbility_Interact::PerformTrace(AMRCharacter* InSourceActor)
{
	TArray<FHitResult> HitResults;
	UCameraComponent* CameraComp = InSourceActor->FindComponentByClass<UCameraComponent>();
	if (CameraComp)
	{
		FVector CameraLocation = CameraComp->GetComponentLocation();
		FVector CameraForward = CameraComp->GetForwardVector();

		float TraceDistance = 1500.f; // 检测射线长度，根据需求调整
		FVector TraceEnd = CameraLocation + CameraForward * TraceDistance;

		FCollisionQueryParams Params;
		Params.bTraceComplex = false;
		Params.AddIgnoredActor(InSourceActor); // 忽略自己

		UWorld* World = InSourceActor->GetWorld();
		if (World && World->SweepMultiByChannel(
			HitResults,CameraLocation,TraceEnd,FQuat::Identity,
			ECC_Visibility,FCollisionShape::MakeSphere(InteractSphereRadius),Params))
		{
			// 命中物体，HitResult包含命中信息
			return HitResults;
		}
		else
		{
			// 未命中任何物体
			UE_LOG(LogTemp, Log, TEXT("No hit"));
			return HitResults;
		}
	}

	return HitResults;
}

// 判断该物体是否实现了InteractableTarget接口,获取接口的最大可交互距离，
// 计算能力发起者与该物体的距离并比较，判断是否可以触发交互事件
bool UMRGameplayAbility_Interact::CanInteractWith(AActor* SourceActor, AActor* TargetToInteract)
{
	if (TargetToInteract && TargetToInteract->GetClass()->ImplementsInterface(UInteractableTarget::StaticClass()))
	{
		IInteractableTarget* target = Cast<IInteractableTarget>(TargetToInteract);
		float MaxInteractDistance = IInteractableTarget::Execute_GetMaxInteractDistance(TargetToInteract);
		float CurrentDistance = FVector::Dist(TargetToInteract->GetActorLocation(), SourceActor->GetActorLocation());
		if (CurrentDistance <= MaxInteractDistance)
		{
			return true;
		}
		else
		{
			UE_LOG(LogMR, Warning, TEXT("false1"));
		}
	}

	return false;
}


void UMRGameplayAbility_Interact::TriggerInteraction(AActor* TargetToInteract)
{
	if (TargetToInteract && TargetToInteract->GetClass()->ImplementsInterface(UInteractableTarget::StaticClass()))
	{
		IInteractableTarget::Execute_OnInteract(TargetToInteract);
	}
}

