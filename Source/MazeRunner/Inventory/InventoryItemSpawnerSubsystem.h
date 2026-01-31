// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "MaruInventoryItemDefinition.h"
#include "InventoryItemSpawnerSubsystem.generated.h"

class UMaruInventoryItemInstance;
class UMaruInventoryItemDefinition;
class APlayerController;

/**
 * 管理Item的生成以及赋予吗？
 * 如果在比如说战斗结束后，赋予玩家奖励要不要通过这个呢？也许功能可以分的更细，但是暂时先用着。
 */
UCLASS()
class MAZERUNNER_API UInventoryItemSpawnerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable,meta = (AutoCreateRefTerm = "TagToInit"))
	UMaruInventoryItemInstance* CreateItemInstanceFromDefinition(TSubclassOf<UMaruInventoryItemDefinition> ItemDef,const TArray<FGameplayTag>& TagToInit);

	//我在想，这个函数在这个类中到底合不合理
	UFUNCTION(BlueprintCallable)
	void GiveToPlayerQuickBar(APlayerController* Player,UMaruInventoryItemInstance* Item);

	UFUNCTION(BlueprintCallable)
	void GiveToAllPlayerQuickBar(UMaruInventoryItemInstance* Item);
};
