// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//#include "Cosmetics/LyraCosmeticAnimationTypes.h"
#include "Equipment/MaruEquipmentInstance.h"
#include "GameFramework/InputDevicePropertyHandle.h"

#include "MaruWeaponInstance.generated.h"


class UAnimInstance;
class UObject;
struct FFrame;
struct FGameplayTagContainer;
class UInputDeviceProperty;

/**
 * UMaruWeaponInstance
 *
 * A piece of equipment representing a weapon spawned and applied to a pawn
 * 随武器装备卸载更改UInputDeviceProperty
 * 记录武器装备和开火的开始时间
 * 提供蓝图访问的接口函数来根据GameplayTag选择合适的动画
 */
UCLASS(MinimalAPI)
class UMaruWeaponInstance : public UMaruEquipmentInstance
{
	GENERATED_BODY()

public:
	 UMaruWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UMaruEquipmentInstance interface
	 virtual void OnEquipped() override;
	 virtual void OnUnequipped() override;
	//~End of UMaruEquipmentInstance interface

	//在LyraGameplayAbility_RangedWeapon::ActivateAbility中被调用
	UFUNCTION(BlueprintCallable)
	 void UpdateFiringTime();

	// Returns how long it's been since the weapon was interacted with (fired or equipped)
	//要这个参数有什么用？
	UFUNCTION(BlueprintPure)
	 float GetTimeSinceLastInteractedWith() const;

protected:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
	//FLyraAnimLayerSelectionSet EquippedAnimSet;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animation)
	//FLyraAnimLayerSelectionSet UneuippedAnimSet;

	/**
	 * Device properties that should be applied while this weapon is equipped.
	 * These properties will be played in with the "Looping" flag enabled, so they will
	 * play continuously until this weapon is unequipped! 
	 * 存储装备武器时应激活的输入设备属性（如手柄震动、灯光颜色、扳机阻力等），
	 * 这些属性以循环（Looping）模式运行，直到武器解除装备。
	 */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Input Devices")
	TArray<TObjectPtr<UInputDeviceProperty>> ApplicableDeviceProperties;
	
	// Choose the best layer from EquippedAnimSet or UneuippedAnimSet based on the specified gameplay tags
	//UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Animation)
	// TSubclassOf<UAnimInstance> PickBestAnimLayer(bool bEquipped, const FGameplayTagContainer& CosmeticTags) const;

	/** Returns the owning Pawn's Platform User ID */
	UFUNCTION(BlueprintCallable)
	 const FPlatformUserId GetOwningUserId() const;

	/** Callback for when the owning pawn of this weapon dies. Removes all spawned device properties. */
	UFUNCTION()
	 void OnDeathStarted(AActor* OwningActor);

	/**
	 * Apply the ApplicableDeviceProperties to the owning pawn of this weapon.
	 * Populate the DevicePropertyHandles so that they can be removed later. This will
	 * Play the device properties in Looping mode so that they will share the lifetime of the
	 * weapon being Equipped.
	 */
	 void ApplyDeviceProperties();

	/** Remove any device proeprties that were activated in ApplyDeviceProperties. */
	 void RemoveDeviceProperties();

private:

	/** Set of device properties activated by this weapon. Populated by ApplyDeviceProperties */
	UPROPERTY(Transient)
	TSet<FInputDevicePropertyHandle> DevicePropertyHandles;

	double TimeLastEquipped = 0.0;
	double TimeLastFired = 0.0;
};

