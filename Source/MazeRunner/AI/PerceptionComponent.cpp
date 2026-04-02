// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/PerceptionComponent.h"
#include "AI/CognitionComponent.h"
#include "MazeRunnerLogChannels.h"

UPerceptionComponent::UPerceptionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void UPerceptionComponent::ReceivePlayerMessageInputFromActor(AActor* Player, const FString& PlayerNickname, const FString& PlayerMessage)
{
	const FString SenderName = !PlayerNickname.IsEmpty() ? PlayerNickname :  TEXT("Unknown");

	UE_LOG(LogAI, Log, TEXT("PerceptionComponent: Receiving player message input from %s"), *SenderName);
	if (!GetOwner())
	{
		return;
	}
	FPerceptionInfo PerceptionInfo;
	PerceptionInfo.Instigator = Player;
	PerceptionInfo.InstigatorName = SenderName;
	PerceptionInfo.Receiver = GetOwner();
	PerceptionInfo.Sense = ESense::Hearing;
	PerceptionInfo.Message = PlayerMessage;
	
	DeliverMessageToCognition(PerceptionInfo);
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
