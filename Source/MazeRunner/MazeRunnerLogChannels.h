// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

DECLARE_LOG_CATEGORY_EXTERN(LogMR, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMRExperience, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMRAbilitySystem, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMRTeams, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogUI, Log, All);

FString GetClientServerContextString(UObject* ContextObject = nullptr);
