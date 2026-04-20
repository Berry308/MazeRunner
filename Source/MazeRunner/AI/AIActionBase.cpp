// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIActionBase.h"
#include "GameFramework/Pawn.h"

void UAIActionBase::BindOwnerPawn(APawn* InOwner)
{
	if(InOwner) OwnerPawn = InOwner;
}

bool UAIActionBase::ConfigureFromParsedParams(const TMap<FString, FString>& RawParams)
{
	return true;
}

void UAIActionBase::Execute_Implementation()
{
	bIsExecuting = true;
}

void UAIActionBase::BeginDestroy()
{
	OnActionFinished.Clear();

	Super::BeginDestroy();
}

