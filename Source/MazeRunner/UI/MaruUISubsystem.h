// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MaruUISubsystem.generated.h"

enum class EWidgetLayer : uint8;
class UActivatableWidget;
/**
 * 
 */
UCLASS()
class MAZERUNNER_API UMaruUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void DeliverWidgetToAllPlayer(EWidgetLayer WidgetLayer, UActivatableWidget* Widget);
};
