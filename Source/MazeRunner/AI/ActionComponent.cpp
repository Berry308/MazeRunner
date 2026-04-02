// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ActionComponent.h"
#include "AI/AIActionBase.h"

void UActionComponent::AddActionToQueue(UAIActionBase* Action)
{
	if (Action)
	{
		ActionList.Add(Action);
	}
}

void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 当前没有执行行为且列表非空时，选最高优先级行为执行，后续或许有打断或插队的机制需要在这里实现
    if (!CurrentAction && ActionList.Num() > 0)
    {
        ActionList.Sort([](const UAIActionBase& A, const UAIActionBase& B) {
            return A.Priority > B.Priority;
            });

        CurrentAction = ActionList[0];
        ActionList.RemoveAt(0);

        if (CurrentAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("开始执行行为，优先级：%d"), CurrentAction->Priority);
            CurrentAction->Execute();
        }
    }
}
