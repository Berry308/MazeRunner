// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NPCManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MAZERUNNER_API UNPCManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	bool GetIsLanguageModelAvailable() const { return bIsLanguageModelAvailable; }
	UFUNCTION()
	void SetIsLanguageModelAvailable(bool isAvailable) { bIsLanguageModelAvailable = isAvailable; }

private:
	bool bIsLanguageModelAvailable = true;
};
