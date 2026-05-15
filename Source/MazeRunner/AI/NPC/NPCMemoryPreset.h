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

    /*以下是推理游戏用到的成员*/
    //NPC知道的信息
	UPROPERTY(EditAnywhere, Category = "DeductionGame")
	TArray<FString> KnownInformation;

	//NPC在案件中扮演的角色
	UPROPERTY(EditAnywhere, Category = "DeductionGame")
	FString RoleInCase;
};
