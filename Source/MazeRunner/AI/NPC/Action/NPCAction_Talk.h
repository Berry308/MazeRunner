// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/NPC/NPCActionBase.h"
#include "NPCAction_Talk.generated.h"

/**
 * 
 */
UCLASS()
class MAZERUNNER_API UNPCAction_Talk : public UNPCActionBase
{
	GENERATED_BODY()
public:
	virtual bool ConfigureFromParsedParams(const TMap<FString, FString>& RawParams) override;

	virtual void Execute_Implementation() override;

public:
	UPROPERTY(EditAnywhere)
	FString TalkContentKey = "Speak";

private:
	FString TalkContent;
};
