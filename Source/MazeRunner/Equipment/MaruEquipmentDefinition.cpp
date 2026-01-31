// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaruEquipmentDefinition.h"
#include "MaruEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaruEquipmentDefinition)

UMaruEquipmentDefinition::UMaruEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = UMaruEquipmentInstance::StaticClass();
}

