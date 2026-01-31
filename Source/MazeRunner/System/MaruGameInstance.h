// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MaruGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MAZERUNNER_API UMaruGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:

	virtual void Init() override;
};
