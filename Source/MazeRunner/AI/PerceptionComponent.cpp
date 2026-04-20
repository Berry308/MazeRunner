// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/PerceptionComponent.h"
#include "AI/CognitionComponent.h"
#include "MazeRunnerLogChannels.h"
#include "Perception/AISenseConfig_Sight.h"

UPerceptionComponent::UPerceptionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//以下配置在编辑器中进行创建
	//// 2. 创建视线配置对象 (Sight Config)
	//UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	//if (SightConfig)
	//{
	//	// 3. 设置视线参数
	//	SightConfig->SightRadius = 2000.f;          // 发现距离
	//	SightConfig->LoseSightRadius = 2500.f;      // 丢失距离
	//	SightConfig->PeripheralVisionAngleDegrees = 60.f; // 视野范围（半角 60度 = 总 120度）
	//	SightConfig->SetMaxAge(1.f);                 // 记忆时长：失去视线后，刺激信息保留1秒

	//	// 4. 重要：设置检测归属（必须包含 Neutral 才能在默认设置下检测到玩家）
	//	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	//	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	//	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	//	// 5. 将视线配置添加到组件中
	//	ConfigureSense(*SightConfig);

	//	// 6. 设置主导感知（可选，通常设为视线）
	//	SetDominantSense(SightConfig->GetSenseImplementation());
	//}
}

bool UPerceptionComponent::ReceivePlayerMessageInputFromActor(AActor* Player, const FString& PlayerNickname, const FString& PlayerMessage)
{
	if (!bIsUseLanguageModel) return false;
	const FString SenderName = !PlayerNickname.IsEmpty() ? PlayerNickname : TEXT("Unknown");

	UE_LOG(LogAI, Log, TEXT("PerceptionComponent: Receiving player message input from %s"), *SenderName);
	if (!GetOwner())
	{
		return false;
	}
	FPerceptionInfo PerceptionInfo;
	PerceptionInfo.Instigator = Player;
	PerceptionInfo.InstigatorName = SenderName;
	PerceptionInfo.Receiver = GetOwner();
	PerceptionInfo.Sense = ESense::Hearing;
	PerceptionInfo.Message = PlayerMessage;

	DeliverMessageToCognition(PerceptionInfo);
	return true;
}

void UPerceptionComponent::DeliverMessageToCognition(const FPerceptionInfo& PerceptionInfo)
{
	UE_LOG(LogAI, Log, TEXT("Delivering message to cognition from %s: %s"), *PerceptionInfo.InstigatorName, *PerceptionInfo.Message);
	UCognitionComponent* CognitionComponent = UCognitionComponent::FindCognitionComponent(PerceptionInfo.Receiver);
	if (CognitionComponent)
	{
		CognitionComponent->ReceivePerceptionMessage(PerceptionInfo);
	}
}

