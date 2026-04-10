// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/DetectionComponent.h"
//#include "Interaction/InteractionStatics.h"
//#include "Interaction/Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
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

//判断当前是否激活了交互能力，以及权威验证，获取当前可交互物体列表中选择的物体
bool UMRGameplayAbility_Interact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	//或许可以设置一个是否激活玩家交互能力的bool值，以防不时之需
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		//判断是否能与某物体交互
		AMRCharacter* Character = Cast<AMRCharacter>(ActorInfo->AvatarActor);
		if (Character)
		{
			return true;
		}
		//else
		//{
		//	UE_LOG(LogMR, Warning, TEXT("%s UMRGameplayAbility_Interact Activate false1"), *ActorInfo->OwnerActor->GetName());
		//}
	}
	//else
	//{
	//	UE_LOG(LogMR, Warning, TEXT("%s UMRGameplayAbility_Interact Activate false2"), *ActorInfo->OwnerActor->GetName());
	//}

	return false;
}

//旧版
//void UMRGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
//{
//	UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s is activating MRGameplayAbility_Interact"), *ActorInfo->OwnerActor->GetName());
//	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
//
//	//判断是否能与某物体交互
//	AMRCharacter* Character = Cast<AMRCharacter>(ActorInfo->AvatarActor);
//	if (Character)
//	{
//		//判断是否指向了物体，做一个扫描检测并返回FHitResult
//		TArray<FHitResult> Hits;
//		Hits = PerformTrace(Character);
//		if (!Hits.IsEmpty())
//		{
//			//判断每个检测到的目标是否能被交互并添加到可交互物体数组中，注意要记得清空
//			for (FHitResult hit : Hits)
//			{
//				AActor* HitTarget = hit.GetActor();
//				if (CanInteractWith(ActorInfo->AvatarActor.Get(), HitTarget))
//				{
//					TargetCanBeInteracted.Add(HitTarget);
//				}
//			}
//		}
//	}
//
//	if (TargetCanBeInteracted.IsEmpty())
//	{
//		UE_LOG(LogMRAbilitySystem, Warning, TEXT("Nothing can be interacted"));
//		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
//	}
//	else
//	{
//		//根据当前选择激活可交互物体，默认交互(后续要添加根据玩家的UI选择来激活对应的可交互物体)
//		if (bIsChosen)
//		{
//			TriggerInteraction();
//		}
//		else
//		{
//			CurrentTargetToInteractIndex = 0;
//			TriggerInteraction();
//		}
//	}
//}

void UMRGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s is activating MRGameplayAbility_Interact"), *ActorInfo->OwnerActor->GetName());
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//判断是否能与某物体交互
	AMRCharacter* Character = Cast<AMRCharacter>(ActorInfo->AvatarActor);
	if (Character)
	{
		//获取Character上的DetectionComponent组件
		UDetectionComponent* DetectionComponent = UDetectionComponent::FindDetectionComponent(Character);
		AActor* TargetToInteract = DetectionComponent->GetCurrentInteractable();
		if (TargetToInteract)
		{
			CurrentTargetToInteract = TargetToInteract;
			TriggerInteraction();
		}
	}
}

//清空所有可交互物体数组
void UMRGameplayAbility_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogMRAbilitySystem, Warning, TEXT("%s 's MRGameplayAbility_Interact is end"), *ActorInfo->OwnerActor->GetName());

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMRGameplayAbility_Interact::TriggerInteraction()
{
	//UE_LOG(LogMRAbilitySystem, Warning, TEXT("MRGameplayAbility_Interact TriggerInteraction"));

	//bTestAlreadyPressed 用于控制任务创建时是否检查输入按键是否已经处于按下状态，如果是true那么会立刻触发按下事件
    //AbilityTask在创建过后会自动激活(此处我使用手动激活，因为实际测试中一直没有激活)，注意在函数内部定义的局部变量在函数结束后会被垃圾回收
	PressTask = UAbilityTask_WaitInputPress::WaitInputPress(this, true);
	PressTask->OnPress.AddDynamic(this, &UMRGameplayAbility_Interact::OnInputPressed);
	PressTask->Activate();

	ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	ReleaseTask->OnRelease.AddDynamic(this, &UMRGameplayAbility_Interact::OnInputReleased);
	ReleaseTask->Activate();

	//if (PressTask->IsActive())
	//{
	//	UE_LOG(LogMRAbilitySystem, Warning, TEXT("PressTask->IsActive"));
	//}
	//if (ReleaseTask->IsActive())
	//{
	//	UE_LOG(LogMRAbilitySystem, Warning, TEXT("ReleaseTask->IsActive"));
	//}
}

void UMRGameplayAbility_Interact::OnInputPressed(float TimeWaited)
{
	//UE_LOG(LogMRAbilitySystem, Warning, TEXT("MRGameplayAbility_Interact OnInputPressed"));

	// 获得发起者角色
	ACharacter* Initiator = Cast<ACharacter>(GetActorInfo().AvatarActor);

	if (CurrentTargetToInteract && Initiator)
	{
		IInteractableTarget::Execute_OnInteract_Press(CurrentTargetToInteract, Initiator);
	}
}

void UMRGameplayAbility_Interact::OnInputHeld(float TimeWaited)
{
	// 获得发起者角色
	ACharacter* Initiator = Cast<ACharacter>(GetActorInfo().AvatarActor);

	if (CurrentTargetToInteract && Initiator)
	{
		IInteractableTarget::Execute_OnInteract_Hold(CurrentTargetToInteract, Initiator);
	}
}

void UMRGameplayAbility_Interact::OnInputReleased(float TimeWaited)
{
	//UE_LOG(LogMRAbilitySystem, Warning, TEXT("MRGameplayAbility_Interact OnInputReleased"));

	// 获得发起者角色
	ACharacter* Initiator = Cast<ACharacter>(GetActorInfo().AvatarActor);

	if (CurrentTargetToInteract && Initiator)
	{
		IInteractableTarget::Execute_OnInteract_Release(CurrentTargetToInteract, Initiator);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

/*TArray<FHitResult> UMRGameplayAbility_Interact::PerformTrace(AMRCharacter* InSourceActor) const
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
}*/

// 判断该物体是否实现了InteractableTarget接口,获取接口的最大可交互距离，
// 计算能力发起者与该物体的距离并比较，判断是否可以触发交互事件
//bool UMRGameplayAbility_Interact::CanInteractWith(AActor* SourceActor, AActor* TargetToInteract) const
//{
//	if (TargetToInteract && TargetToInteract->GetClass()->ImplementsInterface(UInteractableTarget::StaticClass()))
//	{
//		IInteractableTarget* target = Cast<IInteractableTarget>(TargetToInteract);
//		float MaxInteractDistance = IInteractableTarget::Execute_GetMaxInteractDistance(TargetToInteract);
//		float CurrentDistance = FVector::Dist(TargetToInteract->GetActorLocation(), SourceActor->GetActorLocation());
//		if (CurrentDistance <= MaxInteractDistance)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}