// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/NPC/NPCMemoryBase.h"
#include "AI/NPC/NPCMemoryPreset.h"

void UNPCMemoryBase::LoadMemoryPreset(const UNPCMemoryPreset* Preset)
{
	if (Preset)
	{
		PersonalInfo = Preset->PersonalInfo;
		CharacterRelationships = Preset->CharacterRelationships;
		ObjectCognitions = Preset->ObjectCognitions;
		LocationCognitions = Preset->LocationCognitions;
		KnownInformation = Preset->KnownInformation;
		RoleInCase = Preset->RoleInCase;
	}
}
