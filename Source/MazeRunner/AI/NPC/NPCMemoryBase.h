// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NPCMemoryBase.generated.h"

//个人信息，通常不进行更改
USTRUCT(BlueprintType)
struct FMemoryPersonalInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Name;
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Personality;
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Occupation;//职业
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Preferences;//喜好
};

//事件记忆：时间、内容、重要程度
USTRUCT(BlueprintType)
struct FMemoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Memory")
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Content;

    UPROPERTY(EditAnywhere, Category = "Memory")
    int32 Importance = 1; // 记忆重要程度，范围（0-10）
};

//角色关系
USTRUCT(BlueprintType)
struct FMemoryCharacterRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Memory")
    FString OtherCharacterName;
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString RelationshipType; // 关系类型，如朋友、敌人、陌生人等
    UPROPERTY(EditAnywhere, Category = "Memory")
    TArray<FMemoryEvent> Memories; //与这个角色相关的记忆，具有容量上限
    UPROPERTY(EditAnywhere, Category = "Memory")
    float Closeness = 0.5f; // 关系亲密程度
};

//事物认知
USTRUCT(BlueprintType)
struct FMemoryObjectCognition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString ObjectName;
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Description;
    UPROPERTY(EditAnywhere, Category = "Memory")
	TSubclassOf<AActor> ObjectClass; // 事物的类信息
};

//地点认知
USTRUCT(BlueprintType)
struct FMemoryLocationCognition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString LocationName;
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Memory")
    FVector LocationCoordinates; // 地点的坐标信息
};

/**
 * NPC运行时记忆数据
 */

class UNPCMemoryPreset;

UCLASS()
class MAZERUNNER_API UNPCMemoryBase : public USaveGame
{
	GENERATED_BODY()
public:
	//提供一个接口函数，用于从预设数据资产中加载记忆数据
	void LoadMemoryPreset(const UNPCMemoryPreset* Preset);

public:
    //个人信息
	UPROPERTY(VisibleAnywhere, Category = "Memory")
    FMemoryPersonalInfo PersonalInfo;

    //与其它角色的关系
    UPROPERTY(VisibleAnywhere, Category = "Memory")
    TMap<FString, FMemoryCharacterRelationship> CharacterRelationships;

    //事物认知
    UPROPERTY(VisibleAnywhere, Category = "Memory")
	TMap<FString, FMemoryObjectCognition> ObjectCognitions;

    //地点认知
    UPROPERTY(VisibleAnywhere, Category = "Memory")
    TMap<FString, FMemoryLocationCognition> LocationCognitions;

    /*以下是推理游戏用到的成员*/
    //NPC知道的信息
    UPROPERTY(EditAnywhere, Category = "DeductionGame")
    TArray<FString> KnownInformation;

    //NPC在案件中扮演的角色
    UPROPERTY(EditAnywhere, Category = "DeductionGame")
    FString RoleInCase;
};
