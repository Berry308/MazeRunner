// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRCharacter.h"

#include "AbilitySystem/MRAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
//#include "Camera/MRCameraComponent.h"
#include "Character/MRHealthComponent.h"
#include "Character/MRPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MRCharacterMovementComponent.h"
#include "MRGameplayTags.h"
#include "MazeRunnerLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "Player/MRPlayerController.h"
#include "Player/MRPlayerState.h"
//#include "System/MRSignificanceManager.h"//这个是干什么用的
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRCharacter)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

static FName NAME_MRCharacterCollisionProfile_Capsule(TEXT("MRPawnCapsule"));
static FName NAME_MRCharacterCollisionProfile_Mesh(TEXT("MRPawnMesh"));

AMRCharacter::AMRCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMRCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetNetCullDistanceSquared(900000000.0f);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_MRCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_MRCharacterCollisionProfile_Mesh);
	MeshComp->SetOwnerNoSee(true);
	MeshComp->CastShadow = true;
	MeshComp->bCastHiddenShadow = true;
	MeshComp->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	UMRCharacterMovementComponent* MaruMoveComp = CastChecked<UMRCharacterMovementComponent>(GetCharacterMovement());
	MaruMoveComp->GravityScale = 1.0f;
	MaruMoveComp->MaxAcceleration = 2400.0f;
	MaruMoveComp->BrakingFrictionFactor = 1.0f;
	MaruMoveComp->BrakingFriction = 6.0f;
	MaruMoveComp->GroundFriction = 8.0f;
	MaruMoveComp->BrakingDecelerationWalking = 1400.0f;
	MaruMoveComp->bUseControllerDesiredRotation = false;
	MaruMoveComp->bOrientRotationToMovement = false;
	MaruMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MaruMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	MaruMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MaruMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MaruMoveComp->SetCrouchedHalfHeight(65.0f);

	PawnExtComponent = CreateDefaultSubobject<UMRPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UMRHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 90.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	/*FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(FirstPersonCameraComponent);
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 25.0f + BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;*/

	//CameraComponent = CreateDefaultSubobject<ULyraCameraComponent>(TEXT("CameraComponent"));
	//CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void AMRCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();//此处将Character添加到UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);接收器中
}

void AMRCharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

//	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
//	if (bRegisterWithSignificanceManager)
//	{
//		if (ULyraSignificanceManager* SignificanceManager = USignificanceManager::Get<ULyraSignificanceManager>(World))
//		{
////@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
//		}
//	}
}

void AMRCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();

	//const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	//if (bRegisterWithSignificanceManager)
	//{
	//	if (ULyraSignificanceManager* SignificanceManager = USignificanceManager::Get<ULyraSignificanceManager>(World))
	//	{
	//		SignificanceManager->UnregisterObject(this);
	//	}
	//}
}

void AMRCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}

void AMRCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
	DOREPLIFETIME(ThisClass, MyTeamID)*/
}

void AMRCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	//if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	//{
	//	// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
	//	const double MaxAccel = MovementComponent->MaxAcceleration;
	//	const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
	//	double AccelXYRadians, AccelXYMagnitude;
	//	FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

	//	ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
	//	ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
	//	ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	//}
}

void AMRCharacter::NotifyControllerChanged()
{
	//const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our team ID based on the controller
	//if (HasAuthority() && (GetController() != nullptr))
	//{
	//	if (ILyraTeamAgentInterface* ControllerWithTeam = Cast<ILyraTeamAgentInterface>(GetController()))
	//	{
	//		MyTeamID = ControllerWithTeam->GetGenericTeamId();
	//		ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
	//	}
	//}
}

#pragma region GetFunc
AMRPlayerController* AMRCharacter::GetMRPlayerController() const
{
	return CastChecked<AMRPlayerController>(GetController(), ECastCheckedType::NullAllowed);
}

AMRPlayerState* AMRCharacter::GetMRPlayerState() const
{
	return CastChecked<AMRPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UMRAbilitySystemComponent* AMRCharacter::GetMRAbilitySystemComponent() const
{
	return Cast<UMRAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AMRCharacter::GetAbilitySystemComponent() const
{
	if (PawnExtComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtComponent->GetMRAbilitySystemComponent();
}
#pragma endregion


void AMRCharacter::OnAbilitySystemInitialized()
{
	UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent();
	check(MaruASC);

	HealthComponent->InitializeWithAbilitySystem(MaruASC);

	InitializeGameplayTags();
}

void AMRCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

void AMRCharacter::PossessedBy(AController* NewController)
{
	//const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();

	// Grab the current team ID and listen for future changes
	/*if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);*/
}

void AMRCharacter::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	/*const FGenericTeamId OldTeamID = MyTeamID;
	if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}*/

	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();

	// Determine what the new team ID should be afterwards
	/*MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);*/
}

void AMRCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void AMRCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void AMRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

//为ASC设置角色的MovementTag
void AMRCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering(持续的) on the ability system from the previous pawn.
	if (UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : MRGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				MaruASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : MRGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				MaruASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		UMRCharacterMovementComponent* MRMoveComp = CastChecked<UMRCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(MRMoveComp->MovementMode, MRMoveComp->CustomMovementMode, true);
	}
}

#pragma region IGameplayTagAssetInterface
void AMRCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		MaruASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AMRCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		return MaruASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AMRCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		return MaruASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AMRCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		return MaruASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}
#pragma endregion

void AMRCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

//被谁调用
void AMRCharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

void AMRCharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}


void AMRCharacter::DisableMovementAndCollision()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UMRCharacterMovementComponent* MaruMoveComp = CastChecked<UMRCharacterMovementComponent>(GetCharacterMovement());
	MaruMoveComp->StopMovementImmediately();
	MaruMoveComp->DisableMovement();
}

void AMRCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UninitAndDestroy();
}

void AMRCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		if (MaruASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void AMRCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UMRCharacterMovementComponent* MaruMoveComp = CastChecked<UMRCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(MaruMoveComp->MovementMode, MaruMoveComp->CustomMovementMode, true);
}

void AMRCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = nullptr;
		if (MovementMode == MOVE_Custom)
		{
			MovementModeTag = MRGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
		}
		else
		{
			MovementModeTag = MRGameplayTags::MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid())
		{
			MaruASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

void AMRCharacter::ToggleCrouch()
{
	const UMRCharacterMovementComponent* MRMoveComp = CastChecked<UMRCharacterMovementComponent>(GetCharacterMovement());

	if (IsCrouched() || MRMoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (MRMoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void AMRCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		MaruASC->SetLooseGameplayTagCount(MRGameplayTags::Status_Crouching, 1);
	}


	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AMRCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UMRAbilitySystemComponent* MaruASC = GetMRAbilitySystemComponent())
	{
		MaruASC->SetLooseGameplayTagCount(MRGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool AMRCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

//void AMRCharacter::OnRep_ReplicatedAcceleration()
//{
//	if (UMRCharacterMovementComponent* LyraMovementComponent = Cast<UMRCharacterMovementComponent>(GetCharacterMovement()))
//	{
//		// Decompress Acceleration
//		const double MaxAccel         = LyraMovementComponent->MaxAcceleration;
//		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
//		const double AccelXYRadians   = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]
//
//		FVector UnpackedAcceleration(FVector::ZeroVector);
//		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
//		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]
//
//		LyraMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
//	}
//}

//void AMRCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	if (GetController() == nullptr)
//	{
//		if (HasAuthority())
//		{
//			const FGenericTeamId OldTeamID = MyTeamID;
//			MyTeamID = NewTeamID;
//			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//		}
//		else
//		{
//			UE_LOG(LogMRTeams, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
//		}
//	}
//	else
//	{
//		UE_LOG(LogMRTeams, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
//	}
//}
//
//FGenericTeamId AMRCharacter::GetGenericTeamId() const
//{
//	return MyTeamID;
//}
//
//FOnLyraTeamIndexChangedDelegate* AMRCharacter::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}
//
//void AMRCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
//{
//	const FGenericTeamId MyOldTeamID = MyTeamID;
//	MyTeamID = IntegerToGenericTeamId(NewTeam);
//	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
//}
//
//void AMRCharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
//{
//	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//}
//
//bool AMRCharacter::UpdateSharedReplication()
//{
//	if (GetLocalRole() == ROLE_Authority)
//	{
//		FSharedRepMovement SharedMovement;
//		if (SharedMovement.FillForCharacter(this))
//		{
//			// Only call FastSharedReplication if data has changed since the last frame.
//			// Skipping this call will cause replication to reuse the same bunch that we previously
//			// produced, but not send it to clients that already received. (But a new client who has not received
//			// it, will get it this frame)
//			if (!SharedMovement.Equals(LastSharedReplication, this))
//			{
//				LastSharedReplication = SharedMovement;
//				SetReplicatedMovementMode(SharedMovement.RepMovementMode);
//
//				FastSharedReplication(SharedMovement);
//			}
//			return true;
//		}
//	}
//
//	// We cannot fastrep right now. Don't send anything.
//	return false;
//}
//
//void AMRCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
//{
//	if (GetWorld()->IsPlayingReplay())
//	{
//		return;
//	}
//
//	// Timestamp is checked to reject old moves.
//	if (GetLocalRole() == ROLE_SimulatedProxy)
//	{
//		// Timestamp
//		SetReplicatedServerLastTransformUpdateTimeStamp(SharedRepMovement.RepTimeStamp);
//
//		// Movement mode
//		if (GetReplicatedMovementMode() != SharedRepMovement.RepMovementMode)
//		{
//			SetReplicatedMovementMode(SharedRepMovement.RepMovementMode);
//			GetCharacterMovement()->bNetworkMovementModeChanged = true;
//			GetCharacterMovement()->bNetworkUpdateReceived = true;
//		}
//
//		// Location, Rotation, Velocity, etc.
//		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
//		MutableRepMovement = SharedRepMovement.RepMovement;
//
//		// This also sets LastRepMovement
//		OnRep_ReplicatedMovement();
//
//		// Jump force
//		SetProxyIsJumpForceApplied(SharedRepMovement.bProxyIsJumpForceApplied);
//
//		// Crouch
//		if (IsCrouched() != SharedRepMovement.bIsCrouched)
//		{
//			SetIsCrouched(SharedRepMovement.bIsCrouched);
//			OnRep_IsCrouched();
//		}
//	}
//}
//
//FSharedRepMovement::FSharedRepMovement()
//{
//	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
//}
//
//bool FSharedRepMovement::FillForCharacter(ACharacter* Character)
//{
//	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
//	{
//		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
//
//		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
//		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
//		RepMovement.LinearVelocity = CharacterMovement->Velocity;
//		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
//		bProxyIsJumpForceApplied = Character->GetProxyIsJumpForceApplied() || (Character->JumpForceTimeRemaining > 0.0f);
//		bIsCrouched = Character->IsCrouched();
//
//		// Timestamp is sent as zero if unused
//		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
//		{
//			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
//		}
//		else
//		{
//			RepTimeStamp = 0.f;
//		}
//
//		return true;
//	}
//	return false;
//}
//
//bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
//{
//	if (RepMovement.Location != Other.RepMovement.Location)
//	{
//		return false;
//	}
//
//	if (RepMovement.Rotation != Other.RepMovement.Rotation)
//	{
//		return false;
//	}
//
//	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
//	{
//		return false;
//	}
//
//	if (RepMovementMode != Other.RepMovementMode)
//	{
//		return false;
//	}
//
//	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
//	{
//		return false;
//	}
//
//	if (bIsCrouched != Other.bIsCrouched)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool FSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
//{
//	bOutSuccess = true;
//	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
//	Ar << RepMovementMode;
//	Ar << bProxyIsJumpForceApplied;
//	Ar << bIsCrouched;
//
//	// Timestamp, if non-zero.
//	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
//	Ar.SerializeBits(&bHasTimeStamp, 1);
//	if (bHasTimeStamp)
//	{
//		Ar << RepTimeStamp;
//	}
//	else
//	{
//		RepTimeStamp = 0.f;
//	}
//
//	return true;
//}