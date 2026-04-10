// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CognitionComponent.h"
#include "AI/PerceptionComponent.h"
#include "AI/MemoryComponent.h"
#include "AI/ActionComponent.h"
#include "AI/AIActionBase.h"
#include "AI/NPC/NPCMemoryBase.h"
#include "GameFramework/Pawn.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Character/MRCharacter.h"
#include "MazeRunnerLogChannels.h"


UCognitionComponent::UCognitionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

}

void UCognitionComponent::BeginPlay()
{
    MemoryComponent = UMemoryComponent::FindMemoryComponent(GetOwner());
}

void UCognitionComponent::ReceivePerceptionMessage(const FPerceptionInfo& PerceptionInfo)
{
    if (PerceptionInfo.Receiver != GetOwner())
    {
        return;
    }

    const FString SenderName = !PerceptionInfo.InstigatorName.IsEmpty() ? PerceptionInfo.InstigatorName : TEXT("Unknown");
    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Received perception message from %s"), *SenderName);
    CurrentPerception = PerceptionInfo;
    if(MemoryComponent)
    {
        MemoryComponent->SetCurrentInteractType(PerceptionInfo.Instigator);
	}
    AnalyzePerceptionMessage(PerceptionInfo);
}

void UCognitionComponent::AnalyzePerceptionMessage(const FPerceptionInfo& PerceptionInfo)
{
    const FString SenderName = !PerceptionInfo.InstigatorName.IsEmpty() ? PerceptionInfo.InstigatorName : TEXT("Unknown");

    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Analyzing perception message from %s"), *SenderName);
    bool isCallModel = FilterPerceptionMessage(PerceptionInfo);
    if(isCallModel)
    {
        FString Prompt = ConstructPrompt(PerceptionInfo);
        SendPromptToLocalModel(Prompt,1);
    }
}

bool UCognitionComponent::FilterPerceptionMessage(const FPerceptionInfo& PerceptionInfo)
{
    const FString SenderName = !PerceptionInfo.InstigatorName.IsEmpty() ? PerceptionInfo.InstigatorName : TEXT("Unknown");

    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Filtering perception message from %s"), *SenderName);

    return true;
}

FString UCognitionComponent::ConstructMessageFromPerception(const FPerceptionInfo& PerceptionInfo)
{
    FString PerceptionMessage;
    switch (PerceptionInfo.Sense)
    {
    case ESense::Hearing:
        PerceptionMessage = FString::Printf(TEXT("-你听到%s对你说：%s \n"), *PerceptionInfo.InstigatorName, *PerceptionInfo.Message);
        break;
    case ESense::Vision:
        PerceptionMessage = FString::Printf(TEXT("-你看到%s \n"), *PerceptionInfo.Message);
        break;
    case ESense::Smell:
        break;
    case ESense::Tactile:
        break;
	case ESense::Taste:
        break;
    }
	return PerceptionMessage;
}

FString UCognitionComponent::ConstructPrompt(const FPerceptionInfo& PerceptionInfo)
{
    //默认应答约束
    FString Prompt;
    Prompt.Reserve(300);

    // ========== 1. 角色与核心规则 (开头强调) ==========
    Prompt += TEXT("【系统指令】你是一个游戏NPC，必须严格遵循以下规则：\n");
    Prompt += TEXT("1. 你只能从【行为列表】中选择一个行为。\n");
    Prompt += TEXT("2. 你的回答**必须**是一个合法的JSON对象，不包含任何其他文字。\n");
    Prompt += TEXT("3. JSON的格式必须完全参照【输出示例】。\n\n");

    // ========== 2. NPC背景与记忆 (记忆优先) ==========
	//获取记忆模块存储的数据，包括与当前环境信息变化的发起者相关的数据，以及与NPC自身相关的数据，事物和地点认知暂时不加上
    const UNPCMemoryBase* memory;
    if (MemoryComponent)
    {
        memory = MemoryComponent->GetActiveMemoryData();

        if (memory != nullptr)
        {
            //根据记忆模块存储的背景数据，构造身份背景提示词。
			CurrentRelevantMemory = TEXT("【你的角色设定】\n");

            CurrentRelevantMemory.Appendf(TEXT("-姓名:%s\n-性格:%s\n-职业:%s\n-喜好:%s\n"),
                *memory->PersonalInfo.Name,
                *memory->PersonalInfo.Personality,
                *memory->PersonalInfo.Occupation,
                *memory->PersonalInfo.Preferences
            );
			//事物认知和地点认知暂时不加上，后续再根据需要添加
            

            //根据当前环境信息变化的发起者(人或物)，搜索对应记忆模块存储的与发起者相关信息，构造提示词
            if (memory->CharacterRelationships.Find(PerceptionInfo.InstigatorName))
            {
                CurrentRelevantMemory += TEXT("【你的社交关系】\n");

                const FMemoryCharacterRelationship& Relationship = memory->CharacterRelationships[PerceptionInfo.InstigatorName];
                CurrentRelevantMemory.Appendf(TEXT("%s是你的%s"), *Relationship.OtherCharacterName, *Relationship.RelationshipType);
                if (Relationship.Memories.Num() > 0)
                {
                    CurrentRelevantMemory += TEXT(",你们共同经历过的事情有：\n");
                    for (const FMemoryEvent& Event : Relationship.Memories)
                    {
                        CurrentRelevantMemory.Appendf(TEXT("-%s。\n"), *Event.Content);
                    }
                }
            }

            //如果环境信息变化的发起者是事物，搜索记忆中对该事物的认知
            if (memory->ObjectCognitions.Find(PerceptionInfo.InstigatorName))
            {
                CurrentRelevantMemory += TEXT("【你的事物认知】\n");

                const FMemoryObjectCognition& ObjectCognition = memory->ObjectCognitions[PerceptionInfo.InstigatorName];
                CurrentRelevantMemory.Appendf(TEXT("-你对%s的认知是：%s。 \n"),
                    *ObjectCognition.ObjectName,
                    *ObjectCognition.Description
                );
            }

            //应该要加一个短期记忆的提示词构建，需要MemoryComponent来维护
            TArray<FString> ShortTermMemories = MemoryComponent->GetShortTermMemory();
            if (!ShortTermMemories.IsEmpty())
            {
                CurrentRelevantMemory += TEXT("【你最近的记忆信息】\n");
                for (FString s : ShortTermMemories)
                {
                    CurrentRelevantMemory.Appendf(TEXT("-%s\n"),*s);
                }
            }
        }
        else
        {
            CurrentRelevantMemory += TEXT("你失忆了\n");
        }
    }
    Prompt += CurrentRelevantMemory;
    
    // ========== 3. 当前感知信息 ==========
    Prompt += TEXT("【当前发生的事】\n");
    Prompt += ConstructMessageFromPerception(PerceptionInfo);

    UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner());
    if (!ActionComponent)
    {
        UE_LOG(LogAI, Warning, TEXT("%s UCognitionComponent::ConstructPrompt: Get UActionComponent failed"), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
        return Prompt;
    }

    // ========== 4. 行为列表 (清晰化格式) ==========
    Prompt += TEXT("【行为列表】\n");
    const TArray<FAIActionInfo>& AllActionInfos = ActionComponent->GetAllowedActionInfor();
    for (const FAIActionInfo& ActionInfo : AllActionInfos)
    {
        // 明确列出 行为名 和 参数名，并单独给出描述
        Prompt.Appendf(TEXT("- 行为：%s\n"), *ActionInfo.ActionName);
        if (!ActionInfo.Description.IsEmpty())
        {
            Prompt.Appendf(TEXT("  描述：%s\n"), *ActionInfo.Description);
        }
        for (const FAIActionInfoField& Field : ActionInfo.Fields)
        {
            Prompt.Appendf(TEXT("  参数名：%s\n"), *Field.FieldName);
            if (!Field.Description.IsEmpty())
            {
                Prompt.Appendf(TEXT("  参数描述：%s\n"), *Field.Description);
            }
        }
    }

    // ========== 5. 输出格式与示例 (关键部分，清晰且紧跟示例) ==========
    Prompt += TEXT("【输出格式要求】\n");
    Prompt += TEXT("你的回答必须是纯JSON，格式如下：\n");
    Prompt += TEXT("{\"ActionName\": \"[从列表中选择的行为名]\", \"[参数名1]\": \"[你生成的内容]\", ...}\n\n");

    // *** 这是最重要的改进：提供1-2个完整的、简化的示例 ***
    Prompt += TEXT("【输出示例】\n");
    Prompt += TEXT("假设行为列表中有：\n");
    Prompt += TEXT("- 行为：Talk\n  参数名：Speak\n");
    Prompt += TEXT("- 行为：Walk\n  参数名：TargetLocation\n");
    Prompt += TEXT("假设你选择Talk，并且想说“你好”。\n");
    Prompt += TEXT("那么你的回答应该是：\n");
    Prompt += TEXT("{\"ActionName\": \"Talk\", \"Speak\": \"你好\"}\n\n");

    // 如果有更复杂的行为，再加一个示例
    Prompt += TEXT("再例如，如果你选择Walk，目标是“门口”。\n");
    Prompt += TEXT("你的回答：\n");
    Prompt += TEXT("{\"ActionName\": \"Walk\", \"TargetLocation\": \"门口\"}\n\n");

    // ========== 6. 最终指令 (结尾重复规则) ==========
    Prompt += TEXT("【现在开始】\n");
    Prompt += TEXT("请根据以上所有信息，选择最合理的一个行为，并只输出一个JSON对象。\n");

    return Prompt;
}

//此次事件的内容、重要程度
FString UCognitionComponent::ConstructPromptForMemorySummary(const FString& CurrentPerceptionMessage, const FString& ModelResponse)
{
    //需要模型总结的记忆信息，一般只有人和物
    FString Prompt = TEXT("你是一名游戏NPC，需要总结一次外界感知信息和你做出的应答所带来的记忆信息\n");
    Prompt += FString::Printf(TEXT("%s\n"), *CurrentRelevantMemory);
    Prompt += FString::Printf(TEXT("【当前发生的事】\n%s\n"), *CurrentPerceptionMessage);
    Prompt += FString::Printf(TEXT("【你选择应答的行为】\n%s\n"), *ModelResponse);

    Prompt += TEXT("请你总结此次外界感知和行为应答给你带来的记忆信息（以第二人称总结信息），信息应该简洁明了概括客观事实，不需要任何修饰，20字以内。\n");
    //Prompt += FString::Printf(TEXT("输出的格式需要模仿Json，包含字段名和字段内容。\n"));
    //此处可以根据外界感知选择构建不同的提示词，以匹配不同的记忆内容格式
    //if (CurrentPerception.Instigator->IsA<AMRCharacter>())
    //{
    //    Prompt += FString::Printf(TEXT("字段名有Content，内容是此次事件的概要。字段Importance，内容是此次事件的重要程度（0-10）"));
    //}

    return Prompt;
}

void UCognitionComponent::SendPromptToLocalModel(const FString& Prompt,int8 PromptType)
{
    UE_LOG(LogAI, Warning, TEXT("CognitionComponent: Sending cognition message to local model from %s"), *GetOwner()->GetName());
    UE_LOG(LogAI, Log, TEXT("Prompt:\n%s"), *Prompt);

    // 创建HTTP请求
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(APIUrl);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    // 构建JSON请求体
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("model", ModelName);
    JsonObject->SetStringField("prompt", Prompt);
    JsonObject->SetBoolField("stream", false);  // 非流式响应，简化处理

    // 创建并填充 options 对象
    TSharedPtr<FJsonObject> OptionsObject = MakeShareable(new FJsonObject);
    OptionsObject->SetNumberField("temperature", 0.1);   // 低温度，提高确定性
    OptionsObject->SetNumberField("top_k", 10);          // 限制候选词数量
    OptionsObject->SetNumberField("top_p", 0.9);         // 核采样阈值
    OptionsObject->SetNumberField("num_predict", 128);   // 限制最大生成长度（可选）

    // 将 options 对象设置为根对象的字段
    JsonObject->SetObjectField("options", OptionsObject);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    Request->SetContentAsString(RequestBody);

    // 绑定回调
    switch (PromptType)
    {
        case 1:
            Request->OnProcessRequestComplete().BindUObject(this, &UCognitionComponent::OnActionResponseReceived);
            break;
        case 2:
            Request->OnProcessRequestComplete().BindUObject(this, &UCognitionComponent::OnSinglePerceptMemoryResponseReceived);
            break;
        case 3:
			Request->OnProcessRequestComplete().BindUObject(this, &UCognitionComponent::OnShortTermMemorySummaryResponseReceived);
        default:
            break;
    }
    
    RequestStartTimes.Add(&Request.Get(), FPlatformTime::Seconds());
    // 发送请求
    Request->ProcessRequest();
}

void UCognitionComponent::OnActionResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        //计算从请求发出到收到响应的时间差，以评估模型响应时间
        double* StartTimePtr = RequestStartTimes.Find(Request.Get());
        if (StartTimePtr)
        {
            // 2. 计算差值 (当前时间 - 开始时间)
            double DurationSeconds = FPlatformTime::Seconds() - *StartTimePtr;
            float DurationMilliseconds = DurationSeconds * 1000.0f;

            UE_LOG(LogAI, Warning, TEXT("CognitionComponent::OnActionResponseReceived: Successful. CostTime:%.2f ms"), DurationMilliseconds);

            // 4. 清理 Map 以防内存泄漏
            RequestStartTimes.Remove(Request.Get());
        }

        const FString InnerResponseText = ExtractModelResponse(Response->GetContentAsString());
        if (UAIActionBase* ActionToAdd = ConstructActionInstance(InnerResponseText))
        {
            if (UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner()))
            {
                ActionComponent->AddActionToQueue(ActionToAdd);
            }
            //将模型的应答再与先前的环境信息结合，然后发送给模型做记忆信息总结
            FString MemoryPrompt = ConstructPromptForMemorySummary(ConstructMessageFromPerception(CurrentPerception), InnerResponseText);
            SendPromptToLocalModel(MemoryPrompt, 2);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP请求失败"));
    }
}

void UCognitionComponent::OnSinglePerceptMemoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        double* StartTimePtr = RequestStartTimes.Find(Request.Get());
        if (StartTimePtr)
        {
            // 2. 计算差值 (当前时间 - 开始时间)
            double DurationSeconds = FPlatformTime::Seconds() - *StartTimePtr;
            float DurationMilliseconds = DurationSeconds * 1000.0f;

            UE_LOG(LogAI, Warning, TEXT("CognitionComponent::OnSinglePerceptMemoryResponseReceived: Successful. CostTime:%.2f ms"), DurationMilliseconds);

            // 4. 清理 Map 以防内存泄漏
            RequestStartTimes.Remove(Request.Get());
        }

        const FString InnerResponseText = ExtractModelResponse(Response->GetContentAsString());
        UE_LOG(LogAI, Log, TEXT("CognitionComponent::OnSinglePerceptMemoryResponseReceived： ExtractResponse:%s"),*InnerResponseText);
        //将模型的应答存储到短期记忆中
        if (MemoryComponent)
        {
            MemoryComponent->AddShortTermMemory(InnerResponseText,CurrentPerception.InstigatorName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP请求失败"));
    }
}

void UCognitionComponent::OnShortTermMemorySummaryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        double* StartTimePtr = RequestStartTimes.Find(Request.Get());
        if (StartTimePtr)
        {
            // 2. 计算差值 (当前时间 - 开始时间)
            double DurationSeconds = FPlatformTime::Seconds() - *StartTimePtr;
            float DurationMilliseconds = DurationSeconds * 1000.0f;

            UE_LOG(LogAI, Warning, TEXT("CognitionComponent::OnShortTermMemorySummaryResponseReceived: Successful. CostTime:%.2f ms"), DurationMilliseconds);

            // 4. 清理 Map 以防内存泄漏
            RequestStartTimes.Remove(Request.Get());
        }

        UE_LOG(LogAI, Warning, TEXT("CognitionComponent::OnShortTermMemorySummaryResponseReceived： Successful"));
        const FString InnerResponseText = ExtractModelResponse(Response->GetContentAsString());
        //将模型的应答存储到短期记忆中
        ParseMemoryResponse(InnerResponseText);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP请求失败"));
    }
}

FString UCognitionComponent::ExtractModelResponse(const FString& OriginalAns)
{
    //做第一层解析，获取到模型的response字段
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(OriginalAns);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Ollama返回的JSON包含"response"字段
        FString ResponseText;
        if(JsonObject->TryGetStringField(TEXT("response"), ResponseText))
        {
            ResponseText.ReplaceInline(TEXT("```json"), TEXT(""));
            ResponseText.ReplaceInline(TEXT("```"), TEXT(""));
            return ResponseText;
        }
    }
    return TEXT("（NPC无法理解）");
}

void UCognitionComponent::ParseMemoryResponse(const FString& ResponseText)
{
    UE_LOG(LogAI, Log, TEXT("UCognitionComponent::ParseMemoryResponse： local model original response %s"), *ResponseText);
    //解析模型输出的JSON，找到当前要存储的记忆内容和重要程度
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseText);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ParseMemoryResponse: inner JSON parse failed"));
        return;
    }

    if (!MemoryComponent)
    {
        UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ParseMemoryResponse: no MemoryComponent found"));
		return;
    }
    //尝试从Json中提取Content和Importance字段
    FString Content;
    int32 Importance = 0;
    if (JsonObject->TryGetStringField(TEXT("Content"), Content) && JsonObject->TryGetNumberField(TEXT("Importance"), Importance))
    {
        //将解析得到的记忆内容和重要程度传给记忆组件进行存储
        FString memoryName = MemoryComponent->GetSummarizedShortTermMemoryName();
        MemoryComponent->AddCharacterEventMemory(memoryName,Content, Importance);
    }
    else
    {
        UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ParseMemoryResponse: missing Content or Importance"));
    }
	//根据需要还可以添加对其他类型记忆的解析
}

// 解析内层 JSON（模型在 response 字符串里输出的内容），创建行为实例并填入字段
UAIActionBase* UCognitionComponent::ConstructActionInstance(const FString& ResponseText)
{
    UE_LOG(LogAI, Log, TEXT("UCognitionComponent::ConstructActionInstance: response %s"), *ResponseText);

    //解析模型输出的JSON，找到当前要执行的ActionName
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseText);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: inner JSON parse failed"));
		return nullptr;
	}

	FString ActionName;
	if (!JsonObject->TryGetStringField(TEXT("ActionName"), ActionName))
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: missing ActionName"));
		return nullptr;
	}

	//获取当前可执行的行为列表，找到与模型输出的ActionName匹配的行为信息
    UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner());
    if (!ActionComponent)
    {
        UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: no ActionComponent"));
        return nullptr;
    }

	const TArray<FAIActionInfo>& ActionInfos = ActionComponent->GetAllowedActionInfor();
	const FAIActionInfo* MatchedInfo = nullptr;
	for (const FAIActionInfo& Info : ActionInfos)
	{
		if (Info.ActionName == ActionName)
		{
			MatchedInfo = &Info;
			break;
		}
	}

	if (!MatchedInfo)
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: unknown ActionName %s"), *ActionName);
		return nullptr;
	}

	if (!MatchedInfo->ActionClass)
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: ActionClass not set for %s"), *ActionName);
		return nullptr;
	}

    //创建行为实例，并初始化其信息
	UAIActionBase* Action = NewObject<UAIActionBase>(ActionComponent, MatchedInfo->ActionClass);
	if (!Action)
	{
		return nullptr;
	}

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		Action->BindOwnerPawn(OwnerPawn);
	}

    //从Json中提取当前Action需要的字段，填入FieldMessages
	TMap<FString, FString> FieldMessages;
	for (const FAIActionInfoField& Field : MatchedInfo->Fields)
	{
		FString FieldMessage;
		if (JsonObject->TryGetStringField(Field.FieldName, FieldMessage))
		{
			FieldMessages.Add(Field.FieldName, FieldMessage);
		}
	}

	//调用Action的ConfigureFromParsedParams方法，传入FieldMessages，让Action自己解析需要的字段并进行配置
	if (!Action->ConfigureFromParsedParams(FieldMessages))
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: ConfigureFromParsedParams failed"));
		return nullptr;
	}

	return Action;
}

