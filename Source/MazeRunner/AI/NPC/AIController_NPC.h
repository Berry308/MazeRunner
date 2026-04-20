// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController_NPC.generated.h"

class UPerceptionComponent;
class UActionComponent;

/**
 * 控制NPC的决策模式切换
 * 在玩家进入NPC的一定检测范围内，即会触发NPC决策模式切换为模型代理
 */
UCLASS()
class MAZERUNNER_API AAIController_NPC : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnPerceptionUpdate(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void RestartBehaviorTree();
    UFUNCTION()
    void StopBehaviorTree(FString StopReason);

    void StartLanguageModelDrive();
    void StopLanguageModelDrive();

private:
    bool bIsUsingLanguageModel;

public:
    // 可配置行为树和黑板资产
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* DefaultBlackboard;

    //对于是否一直启用语言模型的选项，用于单个NPC场景，对性能要求高
    UPROPERTY(EditAnywhere, Category = "AI")
    bool bKeepUsingLanguageModel = false;

protected:
    UPROPERTY()
    TObjectPtr<UPerceptionComponent> NPCPerceptionComponent;

    UPROPERTY()
    TObjectPtr<UActionComponent> NPCActionComponent;

    //当玩家进入到NPC的感知范围内时，需要经过一段时间才会启用语言模型切换
    UPROPERTY(EditAnywhere, Category = "AI")
    float LanguageModelSwitchDelay = 3.0f;
    // 定时器句柄，用于追踪和取消任务
    FTimerHandle LMSwitchDelayTimerHandle;
private:
    FDelegateHandle OnPerceptionUpdateHandle;
};
