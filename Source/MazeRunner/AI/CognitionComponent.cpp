// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CognitionComponent.h"
#include "AI/PerceptionComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "GameFramework/Pawn.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "AI/ActionComponent.h"
#include "AI/AIActionBase.h"
#include "MazeRunnerLogChannels.h"

UCognitionComponent::UCognitionComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;
}

void UCognitionComponent::ReceivePerceptionMessage(const FPerceptionInfo& PerceptionInfo)
{
    if (PerceptionInfo.Receiver != GetOwner())
    {
        return;
    }
    const FString SenderName = !PerceptionInfo.InstigatorName.IsEmpty() ? PerceptionInfo.InstigatorName : TEXT("Unknown");

    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Received perception message from %s"), *SenderName);
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
        SendPromptToLocalModel(Prompt);
    }
}

bool UCognitionComponent::FilterPerceptionMessage(const FPerceptionInfo& PerceptionInfo)
{
    const FString SenderName = !PerceptionInfo.InstigatorName.IsEmpty() ? PerceptionInfo.InstigatorName : TEXT("Unknown");

    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Filtering perception message from %s"), *SenderName);

    return true;
}

FString UCognitionComponent::ConstructPrompt(const FPerceptionInfo& PerceptionInfo)
{
    //默认应答约束
    FString Prompt = FString::Printf(TEXT("你要扮演游戏中的一名NPC\n"));

    //根据记忆模块存储的背景数据，构造身份背景提示词。为了提高记忆检索速度，需要一些方法如：算法和数据分类，检索出来的数据要与当前环境因素相关
    Prompt += FString::Printf(TEXT("你是一个魔法药水店的女老板。\n"));

    //根据感知组件传递的信息，构造提示词
    switch (PerceptionInfo.Sense)
    {
        case ESense::Hearing:
            Prompt += FString::Printf(TEXT("你听到%s对你说：%s \n"), *PerceptionInfo.InstigatorName, *PerceptionInfo.Message);
            break;
        case ESense::Vision:
            Prompt += FString::Printf(TEXT("你看到%s \n"), *PerceptionInfo.Message);
            break;
    }

    Prompt += FString::Printf(TEXT("你需要根据以上背景和环境信息从提供的选项中选择应答的行为。你必须只输出一个 JSON 对象,以下列表将会是:行为昵称;行为描述;字段:字段描述(可能有多个字段)。你输出的Json格式应该包含字段ActionName(行为昵称),以及该Action要求的其它字段及其内容 \n"));

    //提供可供选择的行为
    Prompt += FString::Printf(TEXT("以下是你可以选择的行为列表：\n"));
    UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner());
    if (!ActionComponent)
    {
        UE_LOG(LogAI, Warning, TEXT("%s UCognitionComponent::ConstructPrompt: Get UActionComponent failed"), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
        return Prompt;
    }

    const TArray<FAIActionInfo>& AllActionInfos = ActionComponent->GetAllowedActionInfor();

    for (const FAIActionInfo& ActionInfo : AllActionInfos)
    {
        Prompt += FString::Printf(TEXT("ActionName: %s; ActionDescription: %s; "), *ActionInfo.ActionName, *ActionInfo.Description);
        if(ActionInfo.Fields.Num() > 0)
        {
            for(const FAIActionInfoField& Field : ActionInfo.Fields)
            {
                Prompt += FString::Printf(TEXT("FieldName: %s; FieldDescription: %s; "), *Field.FieldName, *Field.Description);
            }
		}
		Prompt += FString::Printf(TEXT("\n"));
	}

    return Prompt;
}

void UCognitionComponent::SendPromptToLocalModel(const FString& Prompt)
{
    UE_LOG(LogAI, Log, TEXT("CognitionComponent: Sending cognition message to local model from %s"), *GetOwner()->GetName());
    UE_LOG(LogAI,Log,TEXT("Prompt:\n%s"),*Prompt);

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
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    Request->SetContentAsString(RequestBody);
    
    // 绑定回调
    Request->OnProcessRequestComplete().BindUObject(this, &UCognitionComponent::OnResponseReceived);
    
    // 发送请求
    Request->ProcessRequest();
}

void UCognitionComponent::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        const FString InnerResponseText = ParseModelResponse(Response->GetContentAsString());
        if (UAIActionBase* ActionToAdd = ConstructActionInstance(InnerResponseText))
        {
            if (UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner()))
            {
                ActionComponent->AddActionToQueue(ActionToAdd);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP请求失败"));
    }
}

//解析模型应答中的response字段
FString UCognitionComponent::ParseModelResponse(const FString& OriginalAns)
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
            return ResponseText;
        }
    }
    return TEXT("（NPC无法理解）");
}

// 解析内层 JSON（模型在 response 字符串里输出的内容），创建行为实例并填入字段
UAIActionBase* UCognitionComponent::ConstructActionInstance(const FString& ResponseText)
{
	UActionComponent* ActionComponent = UActionComponent::FindActionComponent(GetOwner());
	if (!ActionComponent)
	{
		UE_LOG(LogAI, Warning, TEXT("UCognitionComponent::ConstructActionInstance: no ActionComponent"));
		return nullptr;
	}

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