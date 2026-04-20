// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/NPC/AIController_NPC.h"
#include "AI/CognitionComponent.h"
#include "AI/PerceptionComponent.h"
#include "AI/ActionComponent.h"
#include "BrainComponent.h"
#include "Perception/AISense_Sight.h"
#include "NPCManagerSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MazeRunnerLogChannels.h"

void AAIController_NPC::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (DefaultBehaviorTree)
    {
        // 启动行为树（会自动初始化黑板）
        RunBehaviorTree(DefaultBehaviorTree);
    }
    if (NPCPerceptionComponent = UPerceptionComponent::FindPerceptionComponent(InPawn))
    {
        NPCPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIController_NPC::OnPerceptionUpdate);
    }
    if (NPCActionComponent = UActionComponent::FindActionComponent(InPawn))
    {
        NPCActionComponent->OnActionListEmpty.AddDynamic(this, &AAIController_NPC::RestartBehaviorTree);
    }

    SetPerceptionComponent(*NPCPerceptionComponent);
    TArray<AActor*> PerceivedActors;
    // 获取当前所有被感知的 Actor
    NPCPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    if (!PerceivedActors.IsEmpty())
    {
        for (AActor* Actor : PerceivedActors)
        {
            if (APawn* CurPawn = Cast<APawn>(Actor))
            {
                if (CurPawn->IsPlayerControlled())
                {
                    // 如果初始就在范围内，手动触发你的交互逻辑
                    StartLanguageModelDrive();
                    break;
                }
            }
        }
    }
}

void AAIController_NPC::OnUnPossess()
{
    NPCPerceptionComponent = nullptr;
    NPCActionComponent = nullptr;
    Super::OnUnPossess();
}

void AAIController_NPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (NPCPerceptionComponent)
    {
        NPCPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAIController_NPC::OnPerceptionUpdate);
    }
    if (NPCActionComponent)
    {
        NPCActionComponent->OnActionListEmpty.RemoveDynamic(this, &AAIController_NPC::RestartBehaviorTree);
    }
    Super::EndPlay(EndPlayReason);
}

void AAIController_NPC::OnPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
    //判断Actor是否为玩家
    if (APawn* CurPawn = Cast<APawn>(Actor))
    {
        if (CurPawn->IsPlayerControlled())
        {
            if (Stimulus.WasSuccessfullySensed())//玩家进入NPC检测范围内，NPC需要等待一段时间后才启用模型控制
            {
                UE_LOG(LogTemp, Log, TEXT("Player detected: %s"), *Actor->GetName());
                //延迟启用NPC的本地模型Agent进行决策和行为选择
                //GetWorldTimerManager().SetTimer(
                //    LMSwitchDelayTimerHandle,
                //    this,
                //    &AAIController_NPC::StartLanguageModelDrive,
                //    LanguageModelSwitchDelay,//默认为3s
                //    false
                //);
            }
            else//玩家离开NPC检测范围内时，清除计时器
            {
                if (LMSwitchDelayTimerHandle.IsValid())
                {
                    GetWorldTimerManager().ClearTimer(LMSwitchDelayTimerHandle);
                }
                StopLanguageModelDrive();
            }
        }
    }
}

void AAIController_NPC::RestartBehaviorTree()
{
	UE_LOG(LogAI, Warning, TEXT("AAIController_NPC::RestartBehaviorTree"));
    if (GetBrainComponent()) 
    {
        GetBrainComponent()->RestartLogic(); 
    }
}

void AAIController_NPC::StopBehaviorTree(FString StopReason)
{
	UE_LOG(LogAI, Warning, TEXT("AAIController_NPC::StopBehaviorTree, reason: %s"), *StopReason);
    if (GetBrainComponent())
    {
        GetBrainComponent()->StopLogic(StopReason);
    }
}

void AAIController_NPC::StartLanguageModelDrive()
{
    //在启用前，判断当前是否有空闲模型可以请求
    UNPCManagerSubsystem* NPCManager = GetGameInstance()->GetSubsystem<UNPCManagerSubsystem>();
    if (NPCManager->GetIsLanguageModelAvailable())
    {
        UE_LOG(LogAI, Warning, TEXT("AAIController_NPC::StartLanguageModelDrive"));
        NPCManager->SetIsLanguageModelAvailable(false);
        bIsUsingLanguageModel = true;
        //停止行为树，并启用NPC的Agent
        StopBehaviorTree(TEXT("SwitchControlMode to SLM"));
        if (NPCPerceptionComponent)
        {
            NPCPerceptionComponent->SetIsUseLanguageModel(true);
            //TODO:此时需要主动传递NPC遇到玩家的感知信息给CognitionComponent让其调用模型作出应答
            
        }
    }
    //else
    //{
    //    UE_LOG(LogAI, Log, TEXT("There is no available languagemodel for %s"), *GetPawn()->GetName());
    //}
}

//注意，在ActionComponent中还有尚待执行的行为时，NPC不能使用行为树进行决策，此函数只是释放了语言模型占用
void AAIController_NPC::StopLanguageModelDrive()
{
    UNPCManagerSubsystem* NPCManager = GetGameInstance()->GetSubsystem<UNPCManagerSubsystem>();
    check(NPCManager);
    NPCManager->SetIsLanguageModelAvailable(true);

    UE_LOG(LogAI, Warning, TEXT("AAIController_NPC::StopLanguageModelDrive"));
    bIsUsingLanguageModel = false;
    if (NPCPerceptionComponent)
    {
        NPCPerceptionComponent->SetIsUseLanguageModel(false);
    }
    if(NPCActionComponent && !NPCActionComponent->IsExecutingAction())
    {
        RestartBehaviorTree();
	}
}
