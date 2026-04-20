// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "PerceptionComponent.generated.h"

/**
 * 感知组件
 *  感知外界信息，并传递给认知组件
 *  外界信息应该分为多个种类，听觉信息（如玩家的文字信息，声音）触觉信息、视觉信息、味觉信息和嗅觉信息
 *  信息包括：发起者，信息种类，具体的字符串信息
 */
UENUM(BlueprintType)
enum class ESense : uint8
{
	Hearing,
	Tactile,//触觉
	Vision,
	Taste,
	Smell
};

USTRUCT(BlueprintType)
struct FPerceptionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Instigator;
	// 发起者昵称/姓名（用于发起者 Actor 销毁后仍可追溯交互来源）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstigatorName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Receiver;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESense Sense;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;//具体的字符串信息
};

UCLASS(Meta=(BlueprintSpawnableComponent))
class MAZERUNNER_API UPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()
	

public:
	UPerceptionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
	static UPerceptionComponent* FindPerceptionComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UPerceptionComponent>() : nullptr; }

	//PerceptionComponent只要不调用使用语言模型的函数，就可以达到禁用语言模型的效果
	UFUNCTION(BlueprintCallable)
	void SetIsUseLanguageModel(bool isUseLanguageModel) { bIsUseLanguageModel = isUseLanguageModel; }
	UFUNCTION(BlueprintPure)
	bool GetIsUseLanguageModel() { return bIsUseLanguageModel; }


#pragma region LanguageModelDrive
public:
	//接收带发起者信息的玩家消息：在发起者销毁后仍能追溯交互来源
	UFUNCTION(BlueprintCallable)
	bool ReceivePlayerMessageInputFromActor(AActor* Player, const FString& PlayerNickname, const FString& PlayerMessage);

protected:
	//将感知到的信息传递给Cognition组件
	UFUNCTION(BlueprintCallable)
	void DeliverMessageToCognition(const FPerceptionInfo& PerceptionInfo);

private:
	bool bIsUseLanguageModel=false;
#pragma endregion
};
