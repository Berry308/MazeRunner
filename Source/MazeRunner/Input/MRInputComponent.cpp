// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/MRLocalPlayer.h"
//#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRInputComponent)

class UMRInputConfig;

UMRInputComponent::UMRInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UMRInputComponent::AddInputMappings(const UMRInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UMRInputComponent::RemoveInputMappings(const UMRInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UMRInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
