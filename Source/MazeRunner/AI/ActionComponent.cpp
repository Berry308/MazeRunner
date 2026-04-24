// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ActionComponent.h"
#include "AI/AIActionBase.h"

void UActionComponent::AddActionToQueue(UAIActionBase* Action)
{
	if (Action)
	{
		ActionList.Add(Action);
	}

    UpdateActionQueue();
}

void UActionComponent::UpdateActionQueue()
{
    // 当前没有执行行为且列表非空时，选最高优先级行为执行。（后续或许有打断或插队的机制需要在这里实现）
    if (!CurrentAction && ActionList.Num() > 0)
    {
        //可以判断CurrentAction是否可抢占
        ActionList.Sort([](const UAIActionBase& A, const UAIActionBase& B) {
            return A.Priority > B.Priority;
            });

        CurrentAction = ActionList[0];
        CurrentAction->OnActionFinished.AddDynamic(this, &UActionComponent::OnCurrentActionFinished);
        ActionList.RemoveAt(0);

        if (CurrentAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("开始执行行为:%s 优先级:%d"), *CurrentAction->GetName(), CurrentAction->Priority);
            CurrentAction->Execute();
        }
    }
    else if (ActionList.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UActionComponent::UpdateActionQueue: 当前行为列表为空"));
        OnActionListEmpty.Broadcast();
    }
}

void UActionComponent::OnCurrentActionFinished()
{
    if (CurrentAction)
    {
        CurrentAction->OnActionFinished.RemoveDynamic(this, &UActionComponent::OnCurrentActionFinished);
        CurrentAction = nullptr;
    }
    UpdateActionQueue();
}

//清空行动队列
void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    OnActionListEmpty.Clear();

	Super::EndPlay(EndPlayReason);
}
