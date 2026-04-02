// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "CognitionComponent.generated.h"

class UAIActionBase;
struct FPerceptionInfo;

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

	//接收感知组件传递的信息
	UFUNCTION(BlueprintCallable)
	void ReceivePerceptionMessage(const FPerceptionInfo& PerceptionInfo);

protected:
	//分析感知组件传递的信息，筛选信息，更改认知(记忆)状态，并发送给本地模型
	void AnalyzePerceptionMessage(const FPerceptionInfo& PerceptionInfo);

	//筛选感知组件传递的信息，判断是否需要调用模型
	bool FilterPerceptionMessage(const FPerceptionInfo& PerceptionInfo);
	
	//根据记忆模块的存储数据和感知组件传递的信息，构造提示词
	FString ConstructPrompt(const FPerceptionInfo& PerceptionInfo);

	//发送认知信息给本地模型
	void SendPromptToLocalModel(const FString& Prompt);

	//当接收到模型的应答时，对其进行解析
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	//解析模型应答的response字段
	FString ParseModelResponse(const FString& JsonString);

	//解析内层 JSON（模型输出的 ActionName 与字段）并创建行为实例
	UAIActionBase* ConstructActionInstance(const FString& ResponseText);

private:
    // 可配置的模型名称（可在蓝图或细节面板中设置）
    UPROPERTY(EditAnywhere, Category = "LLM Settings")
    FString ModelName = TEXT("qwen2:1.5b");

    // Ollama API地址
    FString APIUrl = TEXT("http://localhost:11434/api/generate");
};
