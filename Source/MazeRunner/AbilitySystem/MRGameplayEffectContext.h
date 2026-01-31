// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"

#include "MRGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class IMRAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

//或许需要在项目内配置?
USTRUCT()
struct FMRGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FMRGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FMRGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FMRGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static FMRGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const IMRAbilitySourceInterface* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const IMRAbilitySourceInterface* GetAbilitySource() const;

	//返回带有当前HitResult的FMRGameplayEffectContext副本
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FMRGameplayEffectContext* NewContext = new FMRGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FMRGameplayEffectContext::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** Returns the physical material from the hit result if there is one */
	const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID = -1;

protected:
	/** Ability Source object (should implement IMRAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template<>
struct TStructOpsTypeTraits<FMRGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMRGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

