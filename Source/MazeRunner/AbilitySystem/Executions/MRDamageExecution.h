// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "MRDamageExecution.generated.h"

class UObject;


/**
 * UMRDamageExecution
 *
 *	Execution used by gameplay effects to apply damage to the health attributes.
 *  将CombatSet中的BaseDamage根据距离衰减、物理衰减、以及允许的伤害交互乘数计算最终要造成的伤害
 *  然后通过OutExecutionOutput.AddOutputModifier()指定HealthSet中的Damage属性并修改
 */
UCLASS()
class UMRDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UMRDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
