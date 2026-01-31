// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
//#include "Teams/LyraTeamAgentInterface.h"

#include "MRCharacter.generated.h"


class AActor;
class AController;
class AMRPlayerController;
class AMRPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class UMRAbilitySystemComponent;
class UCameraComponent;
//class UMRCameraComponent;
class UMRHealthComponent;
class UMRPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

#pragma region ReplicatedAcceleration
/**
 * FLyraReplicatedAcceleration: Compressed representation of acceleration
 */
 //USTRUCT()
 //struct FLyraReplicatedAcceleration
 //{
 //	GENERATED_BODY()
 //
 //	UPROPERTY()
 //	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]
 //
 //	UPROPERTY()
 //	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]
 //
 //	UPROPERTY()
 //	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
 //};

 /** The type we use to send FastShared movement updates. */
 //USTRUCT()
 //struct FSharedRepMovement
 //{
 //	GENERATED_BODY()
 //
 //	FSharedRepMovement();
 //
 //	bool FillForCharacter(ACharacter* Character);
 //	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;
 //
 //	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
 //
 //	UPROPERTY(Transient)
 //	FRepMovement RepMovement;
 //
 //	UPROPERTY(Transient)
 //	float RepTimeStamp = 0.0f;
 //
 //	UPROPERTY(Transient)
 //	uint8 RepMovementMode = 0;
 //
 //	UPROPERTY(Transient)
 //	bool bProxyIsJumpForceApplied = false;
 //
 //	UPROPERTY(Transient)
 //	bool bIsCrouched = false;
 //};

 //template<>
 //struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
 //{
 //	enum
 //	{
 //		WithNetSerializer = true,
 //		WithNetSharedSerialization = true,
 //	};
 //};
#pragma endregion

/**
 * AMRCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class AMRCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface/*, public ILyraTeamAgentInterface*/
{
	GENERATED_BODY()

public:

	 AMRCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	 AMRPlayerController* GetMRPlayerController() const;

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	 AMRPlayerState* GetMRPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	 UMRAbilitySystemComponent* GetMRAbilitySystemComponent() const;
	 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//~IGameplayTagAssetInterface
	 virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	 virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	 virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	 virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface

	 void ToggleCrouch();

	//~AActor interface
	 virtual void PreInitializeComponents() override;
	 virtual void BeginPlay() override;
	 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	 virtual void Reset() override;
	 virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	 virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	//~APawn interface
	 virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	//~ILyraTeamAgentInterface interface
	 /*virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	 virtual FGenericTeamId GetGenericTeamId() const override;
	 virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;*/
	//~End of ILyraTeamAgentInterface interface

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	/*UFUNCTION(NetMulticast, unreliable)
	 void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);*/

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	/*FSharedRepMovement LastSharedReplication;*/

	/*virtual bool UpdateSharedReplication();*/

protected:
	//被绑定到PawnExtensionComponent上去了，因为是PawnExtensionComponent触发初始化ASC的
	 virtual void OnAbilitySystemInitialized();
	 virtual void OnAbilitySystemUninitialized();

	 virtual void PossessedBy(AController* NewController) override;
	 virtual void UnPossessed() override;

	 virtual void OnRep_Controller() override;
	 virtual void OnRep_PlayerState() override;

	 virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	 //被AMRCharacter::OnAbilitySystemInitialized调用
	 void InitializeGameplayTags();

	 virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	 virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	 virtual void OnDeathFinished(AActor* OwningActor);

	 void DisableMovementAndCollision();
	 void DestroyDueToDeath();
	 void UninitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	 void K2_OnDeathFinished();

	 virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	 void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	 virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	 virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	 //用于内部判断是否能跳跃
	 virtual bool CanJumpInternal_Implementation() const;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeRunner|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMRPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMRHealthComponent> HealthComponent;

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	// 第一人称相机组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MazeRunner|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<ULyraCameraComponent> CameraComponent;

	//UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	//FLyraReplicatedAcceleration ReplicatedAcceleration;

	//UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	//FGenericTeamId MyTeamID;

	//UPROPERTY()
	//FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	// Called to determine what happens to the team ID when possession ends
	//virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	//{
	//	// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
	//	return FGenericTeamId::NoTeam;
	//}

private:
	//UFUNCTION()
	//UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	//UFUNCTION()
	//UE_API void OnRep_ReplicatedAcceleration();

	//UFUNCTION()
	//UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};


