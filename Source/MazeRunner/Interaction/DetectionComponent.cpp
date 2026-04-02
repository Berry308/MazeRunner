// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/DetectionComponent.h"
#include "Camera/CameraComponent.h"
#include "Interaction/IInteractableTarget.h"

// Sets default values for this component's properties
UDetectionComponent::UDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 启动定时器周期调用ScanForInteractables
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(ScanTimerHandle, this, &UDetectionComponent::ScanForInteractables, ScanInterval, true);
	}
}

//每帧检测
void UDetectionComponent::ScanForInteractables()
{
	TObjectPtr<AActor> InSourceActor = GetOwner();
	TArray<FHitResult> HitResults;
	UCameraComponent* CameraComp = InSourceActor->FindComponentByClass<UCameraComponent>();
	if (CameraComp)
	{
		FVector CameraLocation = CameraComp->GetComponentLocation();
		FVector CameraForward = CameraComp->GetForwardVector();

		float TraceDistance = 1000.f; // 检测射线长度，根据需求调整
		FVector TraceEnd = CameraLocation + CameraForward * TraceDistance;

		FCollisionQueryParams Params;
		Params.bTraceComplex = false;
		Params.AddIgnoredActor(InSourceActor); // 忽略自己

		//扫描检测
		UWorld* World = InSourceActor->GetWorld();
		if (World)
		{
			World->SweepMultiByChannel(
				HitResults, CameraLocation, TraceEnd, FQuat::Identity,
				ECC_Visibility, FCollisionShape::MakeSphere(ScanRadius), Params
			);
		}

		//判断每个检测到的目标是否能被交互并添加到可交互物体数组中
		TSet<AActor*> FilteredInteractables;
		for (FHitResult hit : HitResults)
		{
			AActor* HitTarget = hit.GetActor();
			if (CanInteractWith(InSourceActor, HitTarget))
			{
				FilteredInteractables.Add(HitTarget);
			}
		}

		CurrentInteractables = FilteredInteractables.Array();
		if(CurrentInteractables.Num() == 0)
		{
			SelectedIndex = 0;
		}
	}

	// 广播事件刷新UI列表及高亮
	OnInteractableListUpdated.Broadcast(CurrentInteractables);
	OnSelectedIndexChanged.Broadcast(SelectedIndex);
}

void UDetectionComponent::SelectNextInteractable(bool bForward)
{
	int32 Num = CurrentInteractables.Num();
	if (Num == 0) return;

	SelectedIndex = (SelectedIndex + (bForward ? 1 : -1) + Num) % Num;

	OnSelectedIndexChanged.Broadcast(SelectedIndex);
}

// 判断该物体是否实现了InteractableTarget接口,获取接口的最大可交互距离，
// 计算能力发起者与该物体的距离并比较，判断是否可以触发交互事件
bool UDetectionComponent::CanInteractWith(AActor* SourceActor, AActor* TargetToInteract) const
{
	if (TargetToInteract && TargetToInteract->GetClass()->ImplementsInterface(UInteractableTarget::StaticClass()))
	{
		if (!IInteractableTarget::Execute_CanBeInteracted(TargetToInteract)) return false;
		float MaxInteractDistance = IInteractableTarget::Execute_GetMaxInteractDistance(TargetToInteract);
		float CurrentDistance = FVector::Dist(TargetToInteract->GetActorLocation(), SourceActor->GetActorLocation());
		if (CurrentDistance <= MaxInteractDistance)
		{
			return true;
		}
	}

	return false;
}

AActor* UDetectionComponent::GetCurrentInteractable() const
{
	if (CurrentInteractables.IsValidIndex(SelectedIndex))
	{
		return CurrentInteractables[SelectedIndex];
	}
	return nullptr;
}