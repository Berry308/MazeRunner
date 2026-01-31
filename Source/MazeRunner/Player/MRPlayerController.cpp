// Fill out your copyright notice in the Description page of Project Settings.

#include "MRPlayerController.h"

//#include "CommonInputTypeEnum.h"
#include "Components/PrimitiveComponent.h"
#include "MazeRunnerLogChannels.h"
//#include "LyraCheatManager.h"
#include "MRPlayerState.h"
//#include "Camera/LyraPlayerCameraManager.h"
//#include "UI/LyraHUD.h"
#include "AbilitySystem/MRAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "MRGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemGlobals.h"
//#include "CommonInputSubsystem.h"
#include "MRLocalPlayer.h"
#include "GameModes/MRGameState.h"
//#include "Settings/LyraSettingsLocal.h"
//#include "Settings/LyraSettingsShared.h"
//#include "Replays/LyraReplaySubsystem.h"
//#include "ReplaySubsystem.h"
//#include "Development/LyraDeveloperSettings.h"
#include "GameMapsSettings.h"
#if WITH_RPC_REGISTRY
//#include "Tests/LyraGameplayRpcRegistrationComponent.h"
//#include "HttpServerModule.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRPlayerController)

//namespace Lyra
//{
//	namespace Input
//	{
//		static int32 ShouldAlwaysPlayForceFeedback = 0;
//		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("LyraPC.ShouldAlwaysPlayForceFeedback"),
//			ShouldAlwaysPlayForceFeedback,
//			TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
//	}
//}

AMRPlayerController::AMRPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*PlayerCameraManagerClass = ALyraPlayerCameraManager::StaticClass();*/

#if USING_CHEAT_MANAGER
	CheatClass = ULyraCheatManager::StaticClass();
#endif // #if USING_CHEAT_MANAGER
}

void AMRPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AMRPlayerController::BeginPlay()
{
	Super::BeginPlay();
	/*#if WITH_RPC_REGISTRY
	FHttpServerModule::Get().StartAllListeners();
	int32 RpcPort = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("rpcport="), RpcPort))
	{
		ULyraGameplayRpcRegistrationComponent* ObjectInstance = ULyraGameplayRpcRegistrationComponent::GetInstance();
		if (ObjectInstance && ObjectInstance->IsValidLowLevel())
		{
			ObjectInstance->RegisterAlwaysOnHttpCallbacks();
			ObjectInstance->RegisterInMatchHttpCallbacks();
		}
	}
	#endif*/
	SetActorHiddenInGame(false);
}

void AMRPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMRPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.
	//TargetViewRotation 在引擎中默认通过条件 COND_OwnerOnly 进行同步，只同步给拥有该 PlayerController 的客户端。
	//在重放（Replay）和客户端旁观（Spectating）场景中，客户端通常不是拥有者，该属性不会被同步，导致视角数据丢失或同步异常。
	//Lyra项目通过禁用这条默认同步，改由自身自定义的 ReplicatedViewRotation 属性和 PlayerTick 更新机制来同步视角。
	//这样保证了重放和旁观时，客户端能够正确获取并同步视角信息，解决了基类同步的局限性。
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

void AMRPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void AMRPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	/*if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);
		}
	}*/

	AMRPlayerState* MRPlayerState = GetMRPlayerState();

	//这个主要跟CameraManager相关，应该是处理观战回放和控制的视角的Rotation
	if (PlayerCameraManager && MRPlayerState)
	{
		APawn* TargetPawn = PlayerCameraManager->GetViewTargetPawn();
	
		if (TargetPawn)
		{
			// Update view rotation on the server so it replicates
			if (HasAuthority() || TargetPawn->IsLocallyControlled())
			{
				MRPlayerState->SetReplicatedViewRotation(TargetPawn->GetViewRotation());
			}
	
			// Update the target view rotation if the pawn isn't locally controlled
			if (!TargetPawn->IsLocallyControlled())
			{
				MRPlayerState = TargetPawn->GetPlayerState<AMRPlayerState>();
				if (MRPlayerState)
				{
					// Get it from the spectated pawn's player state, which may not be the same as the PC's playerstate
					TargetViewRotation = MRPlayerState->GetReplicatedViewRotation();
				}
			}
		}
	}
}

AMRPlayerState* AMRPlayerController::GetMRPlayerState() const
{
	return CastChecked<AMRPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UMRAbilitySystemComponent* AMRPlayerController::GetMRAbilitySystemComponent() const
{
	const AMRPlayerState* MaruPS = GetMRPlayerState();
	return (MaruPS ? MaruPS->GetMRAbilitySystemComponent() : nullptr);
}

//ALyraHUD* AMRPlayerController::GetMRHUD() const
//{
//	return CastChecked<ALyraHUD>(GetHUD(), ECastCheckedType::NullAllowed);
//}

//bool AMRPlayerController::TryToRecordClientReplay()
//{
//	// See if we should record a replay
//	if (ShouldRecordClientReplay())
//	{
//		if (ULyraReplaySubsystem* ReplaySubsystem = GetGameInstance()->GetSubsystem<ULyraReplaySubsystem>())
//		{
//			APlayerController* FirstLocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
//			if (FirstLocalPlayerController == this)
//			{
//				// If this is the first player, update the spectator player for local replays and then record
//				if (AMRGameState* GameState = Cast<AMRGameState>(GetWorld()->GetGameState()))
//				{
//					GameState->SetRecorderPlayerState(PlayerState);
//
//					ReplaySubsystem->RecordClientReplay(this);
//					return true;
//				}
//			}
//		}
//	}
//	return false;
//}

//bool AMRPlayerController::ShouldRecordClientReplay()
//{
//	UWorld* World = GetWorld();
//	UGameInstance* GameInstance = GetGameInstance();
//	if (GameInstance != nullptr &&
//		World != nullptr &&
//		!World->IsPlayingReplay() &&
//		!World->IsRecordingClientReplay() &&
//		NM_DedicatedServer != GetNetMode() &&
//		IsLocalPlayerController())
//	{
//		FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
//		FString CurrentMap = World->URL.Map;
//
//#if WITH_EDITOR
//		CurrentMap = UWorld::StripPIEPrefixFromPackageName(CurrentMap, World->StreamingLevelsPrefix);
//#endif
//		if (CurrentMap == DefaultMap)
//		{
//			// Never record demos on the default frontend map, this could be replaced with a better check for being in the main menu
//			return false;
//		}
//
//		if (UReplaySubsystem* ReplaySubsystem = GameInstance->GetSubsystem<UReplaySubsystem>())
//		{
//			if (ReplaySubsystem->IsRecording() || ReplaySubsystem->IsPlaying())
//			{
//				// Only one at a time
//				return false;
//			}
//		}
//
//		// If this is possible, now check the settings
//		if (const UMRLocalPlayer* LyraLocalPlayer = Cast<UMRLocalPlayer>(GetLocalPlayer()))
//		{
//			if (LyraLocalPlayer->GetLocalSettings()->ShouldAutoRecordReplays())
//			{
//				return true;
//			}
//		}
//	}
//	return false;
//}

//void AMRPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
//{
//	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
//}

void AMRPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void AMRPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// Unbind from the old player state, if any
	/*FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (ILyraTeamAgentInterface* PlayerStateTeamInterface = Cast<ILyraTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}*/

	// Bind to the new player state, if any
	/*FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (ILyraTeamAgentInterface* PlayerStateTeamInterface = Cast<ILyraTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}*/

	// Broadcast the team change (if it really has)
	//ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

void AMRPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AMRPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AMRPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so LyraASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (AMRPlayerState* MazeRunnerPS = GetPlayerState<AMRPlayerState>())
		{
			if (UMRAbilitySystemComponent* MazeRunnerASC = MazeRunnerPS->GetMRAbilitySystemComponent())
			{
				MazeRunnerASC->RefreshAbilityActorInfo();
				MazeRunnerASC->TryActivateAbilitiesOnSpawn();
			}
		}
	}
}

void AMRPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	//处理用户输入的变化
	/*if (const UMRLocalPlayer* LyraLocalPlayer = Cast<UMRLocalPlayer>(InPlayer))
	{
		ULyraSettingsShared* UserSettings = LyraLocalPlayer->GetSharedSettings();
		UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

		OnSettingsChanged(UserSettings);
	}*/
}

//void AMRPlayerController::OnSettingsChanged(ULyraSettingsShared* InSettings)
//{
//	bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
//}

void AMRPlayerController::AddCheats(bool bForce)
{
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

//void AMRPlayerController::ServerCheat_Implementation(const FString& Msg)
//{
//#if USING_CHEAT_MANAGER
//	if (CheatManager)
//	{
//		UE_LOG(LogMR, Warning, TEXT("ServerCheat: %s"), *Msg);
//		ClientMessage(ConsoleCommand(Msg));
//	}
//#endif // #if USING_CHEAT_MANAGER
//}

//bool AMRPlayerController::ServerCheat_Validate(const FString& Msg)
//{
//	return true;
//}

//void AMRPlayerController::ServerCheatAll_Implementation(const FString& Msg)
//{
//#if USING_CHEAT_MANAGER
//	if (CheatManager)
//	{
//		UE_LOG(LogMR, Warning, TEXT("ServerCheatAll: %s"), *Msg);
//		for (TActorIterator<AMRPlayerController> It(GetWorld()); It; ++It)
//		{
//			AMRPlayerController* LyraPC = (*It);
//			if (LyraPC)
//			{
//				LyraPC->ClientMessage(LyraPC->ConsoleCommand(Msg));
//			}
//		}
//	}
//#endif // #if USING_CHEAT_MANAGER
//}

//bool AMRPlayerController::ServerCheatAll_Validate(const FString& Msg)
//{
//	return true;
//}

void AMRPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

//
void AMRPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UMRAbilitySystemComponent* LyraASC = GetMRAbilitySystemComponent())
	{
		LyraASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

//void AMRPlayerController::OnCameraPenetratingTarget()
//{
//	bHideViewTargetPawnNextFrame = true;
//}

void AMRPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	//if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	//{
	//	for (const FLyraCheatToRun& CheatRow : GetDefault<ULyraDeveloperSettings>()->CheatsToRun)
	//	{
	//		if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
	//		{
	//			ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
	//		}
	//	}
	//}
#endif

	//SetIsAutoRunning(false);
}

//自动奔跑
/*void AMRPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool AMRPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UMRAbilitySystemComponent* LyraASC = GetMRAbilitySystemComponent())
	{
		bIsAutoRunning = LyraASC->GetTagCount(MRGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void AMRPlayerController::OnStartAutoRun()
{
	if (UMRAbilitySystemComponent* LyraASC = GetMRAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(MRGameplayTags::Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}
}

void AMRPlayerController::OnEndAutoRun()
{
	if (UMRAbilitySystemComponent* LyraASC = GetMRAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(MRGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}*/

//不太明确是干什么的
//void AMRPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
//{
//	if (bForceFeedbackEnabled)
//	{
//		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
//		{
//			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
//			if (Lyra::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
//			{
//				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
//				return;
//			}
//		}
//	}
//
//	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
//}

//
void AMRPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// internal helper func to hide all the components
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
				{
					// add every component and all attached children
					for (UPrimitiveComponent* Comp : InComponents)
					{
						if (Comp->IsRegistered())
						{
							OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

							for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
							{
								static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
								UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
								if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
								{
									OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
								}
							}
						}
					}
				};

			//TODO Solve with an interface.  Gather hidden components or something.
			//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// hide pawn's components
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}

		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}
}

//Team相关
/*void AMRPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogMRTeams, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

FGenericTeamId AMRPlayerController::GetGenericTeamId() const
{
	if (const ILyraTeamAgentInterface* PSWithTeamInterface = Cast<ILyraTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

FOnLyraTeamIndexChangedDelegate* AMRPlayerController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}*/

void AMRPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

//////////////////////////////////////////////////////////////////////
// ALyraReplayPlayerController

//void ALyraReplayPlayerController::Tick(float DeltaSeconds)
//{
//	Super::Tick(DeltaSeconds);
//
//	// The state may go invalid at any time due to scrubbing during a replay
//	if (!IsValid(FollowedPlayerState))
//	{
//		UWorld* World = GetWorld();
//
//		// Listen for changes for both recording and playback
//		if (AMRGameState* GameState = Cast<AMRGameState>(World->GetGameState()))
//		{
//			if (!GameState->OnRecorderPlayerStateChangedEvent.IsBoundToObject(this))
//			{
//				GameState->OnRecorderPlayerStateChangedEvent.AddUObject(this, &ThisClass::RecorderPlayerStateUpdated);
//			}
//			if (APlayerState* RecorderState = GameState->GetRecorderPlayerState())
//			{
//				RecorderPlayerStateUpdated(RecorderState);
//			}
//		}
//	}
//}
//
//void ALyraReplayPlayerController::SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds)
//{
//	// Default behavior is to interpolate to TargetViewRotation which is set from APlayerController::TickActor but it's not very smooth
//
//	Super::SmoothTargetViewRotation(TargetPawn, DeltaSeconds);
//}
//
//bool ALyraReplayPlayerController::ShouldRecordClientReplay()
//{
//	return false;
//}
//
//void ALyraReplayPlayerController::RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState)
//{
//	if (NewRecorderPlayerState)
//	{
//		FollowedPlayerState = NewRecorderPlayerState;
//
//		// Bind to when pawn changes and call now
//		NewRecorderPlayerState->OnPawnSet.AddUniqueDynamic(this, &ALyraReplayPlayerController::OnPlayerStatePawnSet);
//		OnPlayerStatePawnSet(NewRecorderPlayerState, NewRecorderPlayerState->GetPawn(), nullptr);
//	}
//}
//
//void ALyraReplayPlayerController::OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn)
//{
//	if (ChangedPlayerState == FollowedPlayerState)
//	{
//		SetViewTarget(NewPlayerPawn);
//	}
//}