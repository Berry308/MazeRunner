// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRPlayerState.h"

#include "AbilitySystem/Attributes/MRCombatSet.h"
#include "AbilitySystem/Attributes/MRHealthSet.h"
#include "AbilitySystem/MRAbilitySet.h"
#include "AbilitySystem/MRAbilitySystemComponent.h"
#include "Character/MRPawnData.h"
#include "Character/MRPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
//#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/MRExperienceManagerComponent.h"
//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff
#include "GameModes/MRGameMode.h"
#include "MazeRunnerLogChannels.h"
#include "MRPlayerController.h"
//#include "Messages/MRVerbMessage.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

const FName AMRPlayerState::NAME_MRAbilityReady("MRAbilitiesReady");

AMRPlayerState::AMRPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EMRPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UMRAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	// 这里的AttributeSet会被自动识别并且添加到ASC中
	HealthSet = CreateDefaultSubobject<UMRHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UMRCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);

	/*MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;*/
}

void AMRPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AMRPlayerState::Reset()
{
	Super::Reset();
}

void AMRPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UMRPawnExtensionComponent* PawnExtComp = UMRPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void AMRPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void AMRPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
		case EMRPlayerConnectionType::Player:
		case EMRPlayerConnectionType::InactivePlayer:
			//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
			// (e.g., for long running servers where they might build up if lots of players cycle through)
			bDestroyDeactivatedPlayerState = true;
			break;
		default:
			bDestroyDeactivatedPlayerState = true;
			break;
	}
	
	SetPlayerConnectionType(EMRPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void AMRPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EMRPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EMRPlayerConnectionType::Player);
	}
}

//从GameMode中获取PawnData并设置
void AMRPlayerState::OnExperienceLoaded(const UMRExperienceDefinition* /*CurrentExperience*/)
{
	if (AMRGameMode* MRGameMode = GetWorld()->GetAuthGameMode<AMRGameMode>())
	{
		if (const UMRPawnData* NewPawnData = MRGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogMR, Error, TEXT("AMRPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void AMRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	/*DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);*/

	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);	
}

FRotator AMRPlayerState::GetReplicatedViewRotation() const
{
	// Could replace this with custom replication
	return ReplicatedViewRotation;
}

//在PlayerController::PlayerTick中被调用,传入的是PlayerCameraManager->TagetPawn->PawnRotation
void AMRPlayerState::SetReplicatedViewRotation(const FRotator& NewRotation)
{
	if (NewRotation != ReplicatedViewRotation)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
		ReplicatedViewRotation = NewRotation;
	}
}

AMRPlayerController* AMRPlayerState::GetMRPlayerController() const
{
	return Cast<AMRPlayerController>(GetOwner());
}

UAbilitySystemComponent* AMRPlayerState::GetAbilitySystemComponent() const
{
	return GetMRAbilitySystemComponent();
}

//初始化ASC->InitAbilityActorInfo，并且绑定OnExperienceLoaded函数
void AMRPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UMRExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UMRExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnMRExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

//将当前成员PawnData设置为传入值，并且将其PawnData->AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr)
void AMRPlayerState::SetPawnData(const UMRPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogMR, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UMRAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_MRAbilityReady);
	
	ForceNetUpdate();
}

void AMRPlayerState::OnRep_PawnData()
{
}

void AMRPlayerState::SetPlayerConnectionType(EMRPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

//void AMRPlayerState::SetSquadID(int32 NewSquadId)
//{
//	if (HasAuthority())
//	{
//		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);
//
//		MySquadID = NewSquadId;
//	}
//}
//
//void AMRPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	if (HasAuthority())
//	{
//		const FGenericTeamId OldTeamID = MyTeamID;
//
//		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
//		MyTeamID = NewTeamID;
//		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
//	}
//	else
//	{
//		UE_LOG(LogMRTeams, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
//	}
//}
//
//FGenericTeamId AMRPlayerState::GetGenericTeamId() const
//{
//	return MyTeamID;
//}
//
//FOnMRTeamIndexChangedDelegate* AMRPlayerState::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}
//
//void AMRPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
//{
//	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//}
//
//void AMRPlayerState::OnRep_MySquadID()
//{
//	//@TODO: Let the squad subsystem know (once that exists)
//}

void AMRPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void AMRPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 AMRPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool AMRPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

//void AMRPlayerState::ClientBroadcastMessage_Implementation(const FMRVerbMessage Message)
//{
//	// This check is needed to prevent running the action when in standalone mode
//	if (GetNetMode() == NM_Client)
//	{
//		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
//	}
//}

