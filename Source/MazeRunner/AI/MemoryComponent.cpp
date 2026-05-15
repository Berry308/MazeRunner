// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MemoryComponent.h"
#include "AI/NPC/NPCMemoryPreset.h"
#include "AI/CognitionComponent.h"
#include "Character/MRCharacter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "MazeRunnerLogChannels.h"
#include "System/MRAssetManager.h"

UMemoryComponent::UMemoryComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
    if (bUseLongTermMemory)
    {
        bUseShortTermMemory = true;
        if (SaveSlotName.IsEmpty())
        {
            UE_LOG(LogAI, Error, TEXT("UMemoryComponent 构造函数中 SaveSlotName 为空或不存在。"));
        }
        LoadMemoryFromDisk(); // 游戏开始时尝试加载
    }
    else
    {
		AsyncLoadMemoryPreset(); // 直接从预设加载到当前记忆数据中
    }
}

//Warn:我没有在结束游戏时对NPC的短期记忆进行保存，这里需要设计一个全局的管理系统用来处理退出游戏时的逻辑
void UMemoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (bUseLongTermMemory)
    {
        SaveMemoryToDisk(); // 游戏结束时保存当前记忆
    }
}

TArray<FMemoryLocationCognition> UMemoryComponent::GetLocationCognition() const
{
    TArray<FMemoryLocationCognition> result;

    if (ActiveMemoryData)
    {
        for (const TPair<FString, FMemoryLocationCognition>& LocationMemory : ActiveMemoryData->LocationCognitions)
        {
            result.Add(LocationMemory.Value);
        }
    }
    if (result.IsEmpty()) UE_LOG(LogAI, Warning, TEXT("UMemoryComponent::GetLocationCognition failed"));

    return result;
}

FString UMemoryComponent::GetSummarizedShortTermMemoryName()
{
    FString result;
    if (SummariedMemoryName.Dequeue(result))
    {
        return result;
    }
    return FString();
}

//添加信息到短期记忆中，根据触发条件选择是否总结短期记忆，并存储到长期记忆中
void UMemoryComponent::AddShortTermMemory(FString MemoryDescription, FString ObjectName)
{
    if (bUseLongTermMemory)
    {
        //如果当前短期记忆关联者与新记忆关联者不同，或者当前短期记忆容量满了，都需要先总结当前的短期记忆
        if (ObjectName != CurrentMemoryRelevantName || ShortTermMemories.Num() >= ShortTermMemoryCapacity)
        {
            //对当前短期记忆进行总结
            SummarizeShortTermMemories();
            CurrentMemoryRelevantName = ObjectName;
        }
    }
    else if (bUseShortTermMemory)
    {
        CurrentMemoryRelevantName = ObjectName;
        if (ShortTermMemories.Num() == ShortTermMemoryCapacity)
        {
			ShortTermMemories.RemoveAt(0); // 移除最旧的记忆
        }
    }


    //总结完毕后再添加新的短期记忆
    ShortTermMemories.Emplace(MemoryDescription);
}

void UMemoryComponent::AddCharacterRelationshipMemory(FString CharacterName, FString RelationshipType, float Closeness)
{
}

void UMemoryComponent::AddCharacterEventMemory(FString CharacterName, FString EventDescription, int32 Importance)
{
	UE_LOG(LogAI, Log, TEXT("AddCharacterEventMemory about %s to %s"), *CharacterName, *GetOwner()->GetName());
    if (ActiveMemoryData->CharacterRelationships.FindOrAdd(CharacterName).Memories.Add({ FDateTime::Now(), EventDescription, Importance }))
    {
		UE_LOG(LogAI, Warning, TEXT("Successfully added character event memory about %s to %s"), *CharacterName, *GetOwner()->GetName());
    }
}

void UMemoryComponent::AddObjectCognitionMemory(FString ObjectName, FString Description,TSubclassOf<AActor> ObjectClass)
{
}

void UMemoryComponent::AddLocationMemory(FString LocationName, FString Description,FVector Position)
{
}

void UMemoryComponent::SummarizeShortTermMemories()
{
    if (ShortTermMemories.Num() == 0) return;
    SummariedMemoryName.Enqueue(CurrentMemoryRelevantName);
    FString Prompt = ConstructPromptForMemorySummary();
	//发送提示词给模型,让CognitionComponent进行模型应答解析，并将解析结果存储到长期记忆中
    if (UCognitionComponent* CognitionComp = UCognitionComponent::FindCognitionComponent(GetOwner()))
    {
        CognitionComp->SendPromptToLocalModel(Prompt, 3);
    }
    ShortTermMemories.Empty();
}

FString UMemoryComponent::ConstructPromptForMemorySummary()
{
    //需要模型总结的记忆信息，一般只有人和物
    FString Prompt = FString::Printf(TEXT("你需要总结游戏中一名NPC的多次外界信息感知和其做出的应答所带来的记忆信息\n"));

    //获取当前相关的记忆
    FString CurrentRelevantMemory;
    const UNPCMemoryBase* memory = GetActiveMemoryData();
    if (memory != nullptr)
    {
        //根据记忆模块存储的背景数据，构造身份背景提示词。
        CurrentRelevantMemory = FString::Printf(TEXT("你的名字是%s，你的性格是%s，你的职业是%s，你喜欢%s。 \n"),
            *memory->PersonalInfo.Name,
            *memory->PersonalInfo.Personality,
            *memory->PersonalInfo.Occupation,
            *memory->PersonalInfo.Preferences
        );
        //事物认知和地点认知暂时不加上，后续再根据需要添加

        //根据当前环境信息变化的发起者(人或物)，搜索对应记忆模块存储的与发起者相关信息，构造提示词
        if (memory->CharacterRelationships.Find(CurrentMemoryRelevantName))
        {
            const FMemoryCharacterRelationship& Relationship = memory->CharacterRelationships[CurrentMemoryRelevantName];
            CurrentRelevantMemory.Appendf(TEXT("%s是你的%s\n"), *Relationship.OtherCharacterName, *Relationship.RelationshipType);
            if (Relationship.Memories.Num() > 0)
            {
                CurrentRelevantMemory += TEXT("你们共同经历过的事情有：\n");
                for (const FMemoryEvent& Event : Relationship.Memories)
                {
                    CurrentRelevantMemory.Appendf(TEXT("-%s。\n"), *Event.Content);
                }
            }
        }

        //如果环境信息变化的发起者是事物，搜索记忆中对该事物的认知
        if (memory->ObjectCognitions.Find(CurrentMemoryRelevantName))
        {
            const FMemoryObjectCognition& ObjectCognition = memory->ObjectCognitions[CurrentMemoryRelevantName];
            CurrentRelevantMemory.Appendf(TEXT("你对%s的认知是：%s。 \n"),
                *ObjectCognition.ObjectName,
                *ObjectCognition.Description
            );
        }
        //事物认知和地点认知暂时不加上，后续再根据需要添加

    }
    else
    {
        CurrentRelevantMemory += FString::Printf(TEXT("你失忆了\n"));
    }
    Prompt += FString::Printf(TEXT("以下是这名NPC的背景及相关记忆信息：\n%s\n"), *CurrentRelevantMemory);
    
    Prompt += FString::Printf(TEXT("请你以该NPC的视角，根据上述背景信息，总结下列记忆信息：\n"));
    if (!ShortTermMemories.IsEmpty())
    {
        Prompt += TEXT("以下是你需要总结的记忆信息：\n");
        for (FString s : ShortTermMemories)
        {
            Prompt.Appendf(TEXT("-%s\n"), *s);
        }
    }

    Prompt += FString::Printf(TEXT("输出的格式需要模仿Json，包含字段名和字段内容。\n"));
    //此处可以根据外界感知的发起者的不同种类选择构建不同的提示词，以匹配不同的记忆内容格式
    if (CurrentInteractObjectType->IsA<AMRCharacter>())
    {
        Prompt += FString::Printf(TEXT("字段名有Content，内容是此次记忆信息的概要（少于50字）。字段Importance，内容是此次事件的重要程度（0-10）"));
    }

    return Prompt;
}

void UMemoryComponent::SaveMemoryToDisk()
{
    if (ActiveMemoryData)
    {
        // 核心：保存到磁盘槽位
        UGameplayStatics::SaveGameToSlot(ActiveMemoryData, SaveSlotName, 0);
        UE_LOG(LogAI, Warning, TEXT("NPC 记忆已存储到槽位: %s"), *SaveSlotName);
    }
}

void UMemoryComponent::LoadMemoryFromDisk()
{
    // 检查是否存在存档
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        ActiveMemoryData = Cast<UNPCMemoryBase>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    }
    else
    {
        UE_LOG(LogAI, Log, TEXT("SaveSlotName:%s does not exist"), *SaveSlotName);
        // 如果没有存档，则创建一个新的空对象，并将记忆预设注入
        AsyncLoadMemoryPreset();
    }
}

void UMemoryComponent::AsyncLoadMemoryPreset()
{
	//UE_LOG(LogAI, Log, TEXT("UMemoryComponent::AsyncLoadMemoryPreset called"));
    // 如果没有存档，则创建一个新的空对象，并将记忆预设注入
    if (!ActiveMemoryData)
    {
        UE_LOG(LogAI, Log, TEXT("UMemoryComponent::ActiveMemoryData is null"));
        ActiveMemoryData = Cast<UNPCMemoryBase>(UGameplayStatics::CreateSaveGameObject(UNPCMemoryBase::StaticClass()));
        // 1. 如果路径根本没设置，直接退出
        if (MemoryPreset.IsNull())
        {
            UE_LOG(LogAI, Warning, TEXT("MemoryPreset is NULL! Please assign asset in Editor."));
            return;
        }
        if (MemoryPreset.IsValid()) // 是否指向一个有效路径，且该资产目前正处于磁盘上，尚未加载到内存中
        {
            //UE_LOG(LogAI, Warning, TEXT("UMemoryComponent::MemoryPreset.IsPending called"));
            FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			// 异步加载 DataAsset 本身，不对其句柄进行保存，因为我们只需要在加载完成的回调中使用它一次，在回调函数完成后，就会自动被GC回收掉
            Streamable.RequestAsyncLoad(MemoryPreset.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UMemoryComponent::OnMemoryPresetLoaded));
        }

        //记忆预设不用软引用持有时的代码
        /*if (MemoryPreset)
        {
            // 这里可以添加将 MemoryPreset 中的数据复制到 ActiveMemoryData 的逻辑
            ActiveMemoryData->AsyncLoadMemoryPreset(MemoryPreset);
            UE_LOG(LogAI, Log, TEXT("NPC 记忆预设已加载到内存组件"));
        }
        else
        {
            UE_LOG(LogAI, Warning, TEXT("UMemoryComponent 没有设置 MemoryPreset，ActiveMemoryData 将是一个空对象"));
        }*/
    }
}

void UMemoryComponent::OnMemoryPresetLoaded()
{
	//UE_LOG(LogAI, Log, TEXT("UMemoryComponent::OnMemoryPresetLoaded called"));
    if (MemoryPreset.IsValid())
    {
        UNPCMemoryPreset* LoadedPreset = MemoryPreset.Get();
        if (LoadedPreset)
        {
			ActiveMemoryData->LoadMemoryPreset(LoadedPreset);
            UE_LOG(LogAI, Log, TEXT("NPC 记忆预设已异步加载到内存组件"));
        }
        else
        {
            UE_LOG(LogAI, Warning, TEXT("UMemoryComponent 无法加载 MemoryPreset，ActiveMemoryData 将是一个空对象"));
        }
	}
}

