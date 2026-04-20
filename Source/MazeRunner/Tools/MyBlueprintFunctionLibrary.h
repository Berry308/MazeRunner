// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MAZERUNNER_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	// 传入Actor，返回经过NavMesh投影后的坐标
	UFUNCTION(BlueprintCallable, Category = "NavUtility")
	static FVector GetProjectedNavLocation(AActor* TargetActor, float SearchRadius = 500.f);
};
