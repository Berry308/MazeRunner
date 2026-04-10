// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Containers/Queue.h"
#include "MemoryComponent.generated.h"

class UNPCMemoryBase;
class UNPCMemoryPreset;
struct FMemoryPersonalInfo;
struct FPerceptionInfo;
/**
 * 
 */
UCLASS(Meta = (BlueprintSpawnableComponent))
class MAZERUNNER_API UMemoryComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
    UMemoryComponent(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    static UMemoryComponent* FindMemoryComponent(AActor* Actor) {return Actor ? Actor->FindComponentByClass<UMemoryComponent>() : nullptr;}

    UFUNCTION(BlueprintCallable)
	const UNPCMemoryBase* GetActiveMemoryData() const { return ActiveMemoryData; }

    UFUNCTION(BlueprintCallable)
    TArray<FString> GetShortTermMemory() const { return ShortTermMemories; }

    UFUNCTION()
    FString GetSummarizedShortTermMemoryName();

    UFUNCTION()
    void SetCurrentInteractType(AActor* Instigator) { CurrentInteractObjectType = Instigator; }

    // 将当前内存中的记忆写入磁盘
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void SaveMemoryToDisk();
    // 从磁盘加载记忆到内存
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void LoadMemoryFromDisk();

    //添加记忆信息的函数
    void AddShortTermMemory(FString MemoryDescription,FString ObjectName);
    //对所有记忆的容量要做出限制
	void AddCharacterRelationshipMemory(FString CharacterName, FString RelationshipType, float Closeness);
	void AddCharacterEventMemory(FString CharacterName, FString EventDescription,int32 Importance);
	void AddObjectCognitionMemory(FString ObjectName, FString Description, TSubclassOf<AActor> ObjectClass);
    void AddLocationMemory(FString LocationName, FString Description,FVector Position);

protected:
	void SummarizeShortTermMemories();
    FString ConstructPromptForMemorySummary();

protected:
    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // 存档槽位名称，每个 NPC 可以有独立的槽位，或者共用一个
    UPROPERTY(EditAnywhere, Category = "Memory")
    FString SaveSlotName;

    UPROPERTY(EditAnywhere, Category = "Memory|Setup")
	int8 ShortTermMemoryCapacity = 10; // 短期记忆容量上限

    //记忆预设（内存优化、初始化速度优化：在第一次运行时才会注入预设，所以这个成员在后续的运行中都用不到，但是仍然会占用内存，后续可以通过软引用优化）
    UPROPERTY(EditAnywhere, Category = "Memory|Setup")
    UNPCMemoryPreset* MemoryPreset;

protected:
    UPROPERTY()
    TObjectPtr<AActor> CurrentInteractObjectType;
	FString CurrentMemoryRelevantName; // 与当前短期记忆相关的角色或事物名称
    TQueue<FString> SummariedMemoryName; //当前等待被总结的短期记忆相关事物名称 
    UPROPERTY()
	TArray<FString> ShortTermMemories; // 短期记忆列表，具有容量上限，初步设定为10条

    // 运行时内存副本，认知模块直接操作这个
    UPROPERTY()
    UNPCMemoryBase* ActiveMemoryData;
};
