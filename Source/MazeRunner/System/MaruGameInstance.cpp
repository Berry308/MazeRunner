// Fill out your copyright notice in the Description page of Project Settings.

#include "System/MaruGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "MRGameplayTags.h"


void UMaruGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(MRGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(MRGameplayTags::InitState_DataAvailable, false, MRGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(MRGameplayTags::InitState_DataInitialized, false, MRGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(MRGameplayTags::InitState_GameplayReady, false, MRGameplayTags::InitState_DataInitialized);
	}
}
