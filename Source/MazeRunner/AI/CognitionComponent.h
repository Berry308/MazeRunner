// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "PerceptionComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "CognitionComponent.generated.h"

class UAIActionBase;
class UMemoryComponent;
/**
 * 认知组件
 *  认知组件是AI的认知系统，负责接收感知组件传递的信息，并进行分析和决策，以及认知状态的存储和更新
 *  认知组件需要根据接收到的信息，进行分析和决策，并传递给行动组件
 */
UCLASS(Meta=(BlueprintSpawnableComponent))
class MAZERUNNER_API UCognitionComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UCognitionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	static UCognitionComponent* FindCognitionComponent(const AActor* Actor){return (Actor ? Actor->FindComponentByClass<UCognitionComponent>() : nullptr);}

	//接收感知组件传递的信息,option:0-正常感知信息，1-推理游戏中的感知信息（需要构造特殊提示词）
	UFUNCTION(BlueprintCallable)
	void ReceivePerceptionMessage(const FPerceptionInfo& PerceptionInfo,int option);

	//发送认知信息给本地模型,PromptType:1-行为选择，2-单条感知记忆总结，3-短期记忆总结
	void SendPromptToLocalModel(const FString& Prompt, int8 PromptType);
	

protected:

	//分析感知组件传递的信息，筛选信息，更改认知(记忆)状态，并发送给本地模型
	void AnalyzePerceptionMessage(const FPerceptionInfo& PerceptionInfo);

	//筛选感知组件传递的信息，判断是否需要调用模型
	bool FilterPerceptionMessage(const FPerceptionInfo& PerceptionInfo);

	//推理游戏中专用的分析函数，构造特殊提示词
	void AnalyzePerceptionMessageForDeductionGame(const FPerceptionInfo& PerceptionInfo);
	
	//从感知信息中提取信息并构建字符串
	FString ConstructMessageFromPerception(const FPerceptionInfo& PerceptionInfo);

	//根据记忆模块的存储数据和感知组件传递的信息，构造提示词
	FString ConstructPrompt(const FPerceptionInfo& PerceptionInfo);
	FString ConstructPromptForMemorySummary(const FString& CurrentPerception, const FString& ModelResponse);
	//NPC扮演推理游戏专用的提示词
	FString ConstructPromptForDeductionGame(const FString& PlayerMessage);

	//当接收到模型的行为选择应答时，对其进行解析
	void OnActionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	//当接收到模型的记忆总结应答时，对其进行解析
	void OnSinglePerceptMemoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnShortTermMemorySummaryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	//提取模型应答的response字段
	FString ExtractModelResponse(const FString& JsonString);
	//解析模型的Memory应答
	void ParseMemoryResponse(const FString& ResponseText);
	//解析内层 JSON（模型输出的 ActionName 与字段）并创建行为实例
	UAIActionBase* ConstructActionInstance(const FString& ResponseText);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMemoryComponent* MemoryComponent;

	//是否开启短期记忆功能
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bEnableShortTermMemory = true;

    /**可配置的模型名称（可在蓝图或细节面板中设置）*/
    UPROPERTY(EditAnywhere, Category = "Settings|Model")
    FString ModelName = TEXT("qwen2:1.5b");

	/*模型温度，取值范围0~1，温度越低确定性越高，角色扮演需要一定温度来使对话具有随机性*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	double Temperature = 0.7;
	/*模型的核采样阈值，取值范围0~1，较高的值会产生更具创造性的输出，而较低的值会产生更确定性的输出。*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	double TopP = 0.9;
	/*模型生成文本的最大长度，单位为 token，超过这个长度模型将停止生成文本。*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	int MaxTokens = 128;
	/*模型的 top-k 采样参数，代表候选词数量，控制生成文本的多样性，取值为正整数，较高的值会产生更具创造性的输出，而较低的值会产生更确定性的输出。*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	int TopK = 40;
	/*是否启用模型的thinking功能*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	bool bDisableThinking = true;
	/*Ollama API地址*/
	UPROPERTY(EditAnywhere, Category = "Settings|Model")
	FString APIUrl = TEXT("http://localhost:11434/api/generate");

	FString CurrentRelevantMemory;

	FPerceptionInfo CurrentPerception;

	// 存储请求与其发出时间的映射
	TMap<IHttpRequest*, double> RequestStartTimes;
};
