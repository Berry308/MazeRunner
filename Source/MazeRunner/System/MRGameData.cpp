// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameData.h"
#include "MRAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MRGameData)

UMRGameData::UMRGameData()
{
}

const UMRGameData& UMRGameData::UMRGameData::Get()
{
	return UMRAssetManager::Get().GetGameData();
}
