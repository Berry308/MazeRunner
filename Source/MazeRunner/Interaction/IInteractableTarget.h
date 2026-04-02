// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
//#include "InteractionOption.h"
#include "IInteractableTarget.generated.h"

#pragma region LyraOriginal
//struct FInteractionQuery;
//
///**  */
//class FInteractionOptionBuilder
//{
//public:
//	FInteractionOptionBuilder(TScriptInterface<IInteractableTarget> InterfaceTargetScope, TArray<FInteractionOption>& InteractOptions)
//		: Scope(InterfaceTargetScope)
//		, Options(InteractOptions)
//	{
//	}
//
//	void AddInteractionOption(const FInteractionOption& Option)
//	{
//		FInteractionOption& OptionEntry = Options.Add_GetRef(Option);
//		OptionEntry.InteractableTarget = Scope;
//	}
//
//private:
//	TScriptInterface<IInteractableTarget> Scope;
//	TArray<FInteractionOption>& Options;
//};

/**  */
//UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
//class UInteractableTarget : public UInterface
//{
//	GENERATED_BODY()
//};
//
///**  */
//class IInteractableTarget
//{
//	GENERATED_BODY()
//
//public:
//	/**  */
//	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& OptionBuilder) = 0;
//
//	/**  */
//	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) { }
//};
#pragma endregion

UENUM(BlueprintType)
enum class EInteractTriggerType : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};


/**
* 定义物体的最大可交互距离,交互触发类型等
* 定义交互类型，是按下触发，还是松开触发，还是长按一定时间后触发
*/
UINTERFACE(MinimalAPI)
class UInteractableTarget : public UInterface
{
	GENERATED_BODY()
};

/**  */
class IInteractableTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	bool CanBeInteracted() const;
	virtual bool CanBeInteracted_Implementation() const { return true;}

	// Getter返回最大交互距离，蓝图/C++都可实现（用BlueprintNativeEvent）
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	float GetMaxInteractDistance() const;

	//在可交互的时候（弹出可交互UI提示），单个可交互目标可能有多个可交互选项（尚不实现）
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	FText GetInteractablePrompt();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void OnInteractable();

	// 交互事件
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void OnInteract_Press(ACharacter* Indicator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void OnInteract_Hold(ACharacter* Indicator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
	void OnInteract_Release(ACharacter* Indicator);
};
