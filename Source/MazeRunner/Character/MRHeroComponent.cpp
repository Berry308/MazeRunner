// Fill out your copyright notice in the Description page of Project Settings.


#include "MRHeroComponent.h"

#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "MazeRunnerLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "Player/MRPlayerController.h"
#include "Player/MRPlayerState.h"
#include "Player/MRLocalPlayer.h"
#include "Character/MRPawnExtensionComponent.h"
#include "Character/MRPawnData.h"
#include "Character/MRCharacter.h"
#include "AbilitySystem/MRAbilitySystemComponent.h"
#include "Input/MRInputConfig.h"
#include "Input/MRInputComponent.h"
//#include "Camera/LyraCameraComponent.h"
#include "MRGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
//#include "Camera/LyraCameraMode.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace MRHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UMRHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UMRHeroComponent::NAME_ActorFeatureName("Hero");

UMRHeroComponent::UMRHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/*AbilityCameraMode = nullptr;*/
	bReadyToBindInputs = false;
}

void UMRHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogMR, Error, TEXT("[UMRHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("MRHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("MRHeroComponent");

			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

//if(Pawn)->InitState_Spawned;
//PS存在且与当前Controller配对，本地控制时需要InputComponent和LocalPlayer -> InitState_DataAvailable
//PS存在，PawnExtensionComponent到达InitState_DataAvailable状态 -> InitState_DataInitialized
//true -> InitState_GameplayReady
bool UMRHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == MRGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == MRGameplayTags::InitState_Spawned && DesiredState == MRGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<AMRPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		//确认当前PawnComponent的拥有者Pawn是被本地控制并且有InputComponent，确保PawnComponent的Controller有本地玩家
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AMRPlayerController* MazeRunnerPC = GetController<AMRPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !MazeRunnerPC || !MazeRunnerPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == MRGameplayTags::InitState_DataAvailable && DesiredState == MRGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		AMRPlayerState* MRPS = GetPlayerState<AMRPlayerState>();

		return MRPS && Manager->HasFeatureReachedInitState(Pawn, UMRPawnExtensionComponent::NAME_ActorFeatureName, MRGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == MRGameplayTags::InitState_DataInitialized && DesiredState == MRGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

//PawnExtComp->InitializeAbilitySystem;InitializePlayerInput(Pawn->InputComponent)
void UMRHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	//UE_LOG(LogMR, Log, TEXT("%s HeroComponent is HandleChangeInitState"), *GetOwningActor()->GetName());

	if (CurrentState == MRGameplayTags::InitState_DataAvailable && DesiredState == MRGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AMRPlayerState* MRPS = GetPlayerState<AMRPlayerState>();
		if (!ensure(Pawn && MRPS))
		{
			return;
		}

		const UMRPawnData* PawnData = nullptr;

		if (UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UMRPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(MRPS->GetMRAbilitySystemComponent(), MRPS);
		}

		if (AMRPlayerController* MRPC = GetController<AMRPlayerController>())
		{
			UE_LOG(LogMR, Log, TEXT("%s HeroComponent is HandleChangeInitState GetController"), *GetOwningActor()->GetName());

			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// Hook up the delegate for all pawns, in case we spectate later
		/*if (PawnData)
		{
			if (UMRCameraComponent* CameraComponent = UMRCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}*/
	}
}

//当PawnExtensionComponent初始化状态变化，并且当前FeatureState为MRGameplayTags::InitState_DataInitialized
void UMRHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UMRPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == MRGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

//检查默认初始化，定义StateChain并持续初始化状态链
void UMRHeroComponent::CheckDefaultInitialization()
{
	//UE_LOG(LogMR, Log, TEXT("%s HeroComponent is CheckDefaultInitialization"), *GetOwningActor()->GetName());
	static const TArray<FGameplayTag> StateChain = { MRGameplayTags::InitState_Spawned, MRGameplayTags::InitState_DataAvailable, MRGameplayTags::InitState_DataInitialized, MRGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UMRHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state,
	BindOnActorInitStateChanged(UMRPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(MRGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UMRHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

//被HandleChangeInitState所调用
void UMRHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogMR, Warning, TEXT("%s HeroComponent is InitializePlayerInput"), *GetOwningActor()->GetName());
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const UMRLocalPlayer* LP = Cast<UMRLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UMRPawnData* PawnData = PawnExtComp->GetPawnData<UMRPawnData>())
		{
			if (const UMRInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}

							//这个是什么玩意？
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}

				// The Lyra Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the UMRInputComponent or modify this component accordingly.
				UMRInputComponent* MazeRunnerIC = Cast<UMRInputComponent>(PlayerInputComponent);
				if (ensureMsgf(MazeRunnerIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UMRInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					MazeRunnerIC->AddInputMappings(InputConfig, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					MazeRunnerIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					MazeRunnerIC->BindNativeAction(InputConfig, MRGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MazeRunnerIC->BindNativeAction(InputConfig, MRGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					MazeRunnerIC->BindNativeAction(InputConfig, MRGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					MazeRunnerIC->BindNativeAction(InputConfig, MRGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					//MazeRunnerIC->BindNativeAction(InputConfig, MRGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
		UE_LOG(LogMR, Log, TEXT("HeroComponent is Ready To Bind Inputs"));
	}

	//注意，这里手动调用触发扩展事件委托,接收者是PlayerController(GFA_AddInputMappingContext,为PC的LocalPlayer的InputSystem添加IMC)和Pawn。
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

//被GameFeatureAction_InputBinding所使用
void UMRHeroComponent::AddAdditionalInputConfig(const UMRInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		UMRInputComponent* MRIC = Pawn->FindComponentByClass<UMRInputComponent>();
		if (ensureMsgf(MRIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UMRInputComponent or a subclass of it.")))
		{
			MRIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}
	}
}

void UMRHeroComponent::RemoveAdditionalInputConfig(const UMRInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

bool UMRHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UMRHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UMRAbilitySystemComponent* MRASC = PawnExtComp->GetMRAbilitySystemComponent())
			{
				MRASC->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void UMRHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMRAbilitySystemComponent* MRASC = PawnExtComp->GetMRAbilitySystemComponent())
		{
			MRASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UMRHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// If the player has attempted to move again then cancel auto running
	/*if (AMRPlayerController* MRController = Cast<AMRPlayerController>(Controller))
	{
		MRController->SetIsAutoRunning(false);
	}*/

	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UMRHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UMRHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * MRHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * MRHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void UMRHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (AMRCharacter* Character = GetPawn<AMRCharacter>())
	{
		Character->ToggleCrouch();
	}
}

//void UMRHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
//{
//	if (APawn* Pawn = GetPawn<APawn>())
//	{
//		if (AMRPlayerController* Controller = Cast<AMRPlayerController>(Pawn->GetController()))
//		{
//			// Toggle auto running
//			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
//		}
//	}
//}

//TSubclassOf<UMRCameraMode> UMRHeroComponent::DetermineCameraMode() const
//{
//	if (AbilityCameraMode)
//	{
//		return AbilityCameraMode;
//	}
//
//	const APawn* Pawn = GetPawn<APawn>();
//	if (!Pawn)
//	{
//		return nullptr;
//	}
//
//	if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
//	{
//		if (const UMRPawnData* PawnData = PawnExtComp->GetPawnData<UMRPawnData>())
//		{
//			return PawnData->DefaultCameraMode;
//		}
//	}
//
//	return nullptr;
//}

//void UMRHeroComponent::SetAbilityCameraMode(TSubclassOf<ULyraCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
//{
//	if (CameraMode)
//	{
//		AbilityCameraMode = CameraMode;
//		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
//	}
//}

//void UMRHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
//{
//	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
//	{
//		AbilityCameraMode = nullptr;
//		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
//	}
//}