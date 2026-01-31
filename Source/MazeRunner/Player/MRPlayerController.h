// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Camera/LyraCameraAssistInterface.h"
//#include "CommonPlayerController.h"
#include "ModularPlayerController.h"
//#include "Teams/LyraTeamAgentInterface.h"

#include "MRPlayerController.generated.h"


//struct FGenericTeamId;

class ALyraHUD;
class AMRPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class UMRAbilitySystemComponent;
//class ULyraSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * AMRPlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class AMRPlayerController : public AModularPlayerController/*, public ILyraCameraAssistInterface, public ILyraTeamAgentInterface*/
{
	GENERATED_BODY()

public:

	 AMRPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "MazeRunner|PlayerController")
	 AMRPlayerState* GetMRPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "MazeRunner|PlayerController")
	 UMRAbilitySystemComponent* GetMRAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "MazeRunner|PlayerController")
	 //ALyraHUD* GetMRHUD() const;

	// Call from game state logic to start recording an automatic client replay if ShouldRecordClientReplay returns true
	/*UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	UE_API bool TryToRecordClientReplay();*/

	// Call to see if we should record a replay, subclasses could change this
	/*UE_API virtual bool ShouldRecordClientReplay();*/

	// Run a cheat command on the server.
	/*UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheat(const FString& Msg);*/

	// Run a cheat command on the server for all players.
	/*UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheatAll(const FString& Msg);*/

	//~AActor interface
	 virtual void PreInitializeComponents() override;
	 virtual void BeginPlay() override;
	 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	 virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AController interface
	 virtual void OnPossess(APawn* InPawn) override;
	 virtual void OnUnPossess() override;
	 virtual void InitPlayerState() override;
	 virtual void CleanupPlayerState() override;
	 virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	 virtual void ReceivedPlayer() override;
	 virtual void PlayerTick(float DeltaTime) override;
	 virtual void SetPlayer(UPlayer* InPlayer) override;
	 virtual void AddCheats(bool bForce) override;
	 //virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	 virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	 virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	 virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	//~ILyraCameraAssistInterface interface
	/*UE_API virtual void OnCameraPenetratingTarget() override;*/
	//~End of ILyraCameraAssistInterface interface

	//~ILyraTeamAgentInterface interface
	//UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	//UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	/*UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API void SetIsAutoRunning(const bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API bool GetIsAutoRunning() const;*/

private:
	/*UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;*/

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

private:
	//UFUNCTION()
	//void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// Called when the player state is set or cleared
	 virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:

	//~APlayerController interface

	//~End of APlayerController interface

	 //void OnSettingsChanged(ULyraSettingsShared* Settings);

	//UE_API void OnStartAutoRun();
	//UE_API void OnEndAutoRun();

	//UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	//UE_API void K2_OnStartAutoRun();

	//UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	//UE_API void K2_OnEndAutoRun();

	bool bHideViewTargetPawnNextFrame = false;
};


// A player controller used for replay capture and playback
//UCLASS()
//class ALyraReplayPlayerController : public AMRPlayerController
//{
//	GENERATED_BODY()
//
//	virtual void Tick(float DeltaSeconds) override;
//	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
//	virtual bool ShouldRecordClientReplay() override;
//
//	// Callback for when the game state's RecorderPlayerState gets replicated during replay playback
//	void RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState);
//
//	// Callback for when the followed player state changes pawn
//	UFUNCTION()
//	void OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn);
//
//	// The player state we are currently following */
//	UPROPERTY(Transient)
//	TObjectPtr<APlayerState> FollowedPlayerState;
//};
