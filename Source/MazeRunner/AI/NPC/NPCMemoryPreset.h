// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCMemoryBase.h"
#include "NPCMemoryPreset.generated.h"

/**
 * 
 */
UCLASS()
class MAZERUNNER_API UNPCMemoryPreset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    //个人信息
    UPROPERTY(EditAnywhere, Category = "Memory")
    FMemoryPersonalInfo PersonalInfo;

    //与其它角色的关系
    UPROPERTY(EditAnywhere, Category = "Memory")
    TMap<FString, FMemoryCharacterRelationship> CharacterRelationships;

    //事物认知
    UPROPERTY(EditAnywhere, Category = "Memory")
    TMap<FString, FMemoryObjectCognition> ObjectCognitions;

    //地点认知
    UPROPERTY(EditAnywhere, Category = "Memory")
    TMap<FString, FMemoryLocationCognition> LocationCognitions;
};
