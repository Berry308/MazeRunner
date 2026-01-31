// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"

#include "MRPlayerSpawningManagerComponent.generated.h"


class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AMRPlayerStart;
class AActor;

/**
 * @class UMRPlayerSpawningManagerComponent
 */
UCLASS(MinimalAPI)
class UMRPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	 UMRPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	 virtual void InitializeComponent() override;
	 virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */

protected:
	// Utility
	 APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AMRPlayerStart*>& FoundStartPoints) const;
	
	//自定义生成逻辑
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AMRPlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	 void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

	/** We proxy these calls from AMRGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	 AActor* ChoosePlayerStart(AController* Player);
	 bool ControllerCanRestart(AController* Player);
	 void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class AMRGameMode;
	/** ~AMRGameMode */

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AMRPlayerStart>> CachedPlayerStarts;

private:
	 void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	 void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	 APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};

