// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameplayEffectContext.h"

#include "AbilitySystem/MRAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
//#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
//#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRGameplayEffectContext)

class FArchive;

FMRGameplayEffectContext* FMRGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FMRGameplayEffectContext::StaticStruct()))
	{
		return (FMRGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FMRGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

#if UE_WITH_IRIS
//namespace UE::Net
//{
//	// Forward to FGameplayEffectContextNetSerializer
//	// Note: If FMRGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
//	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(MRGameplayEffectContext, FGameplayEffectContextNetSerializer);
//}
#endif

void FMRGameplayEffectContext::SetAbilitySource(const IMRAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IMRAbilitySourceInterface* FMRGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IMRAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FMRGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

