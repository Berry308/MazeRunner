// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/NPC/Action/NPCAction_Talk.h"
#include "UI/SpeakInworldComponent.h"

bool UNPCAction_Talk::ConfigureFromParsedParams(const TMap<FString, FString>& RawParams)
{
	if (const FString* ContentPtr = RawParams.Find(TalkContentKey))
	{
		TalkContent = *ContentPtr;
		return true;
	}
	return false;
}

void UNPCAction_Talk::Execute_Implementation()
{
	Super::Execute_Implementation();
	USpeakInworldComponent* SpeakComp = GetOwnerPawn()->FindComponentByClass<USpeakInworldComponent>();
	if (SpeakComp)
	{
		SpeakComp->AddSpeakContentAndSpeak(TalkContent);
	}
	else UE_LOG(LogTemp, Error, TEXT("UNPCAction_Talk: Owner Pawn %s does not have a USpeakInworldComponent!"), *GetOwnerPawn()->GetName());

	OnActionFinished.Broadcast();
}
