// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SpeakInworldComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

// Sets default values for this component's properties
USpeakInworldComponent::USpeakInworldComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 创建内部 Widget 组件并附加到此 SceneComponent
    InternalWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InternalWidget"));
    InternalWidgetComp->SetupAttachment(this);

    // 配置 Widget 组件属性
    InternalWidgetComp->SetWidgetSpace(EWidgetSpace::World);
    InternalWidgetComp->SetDrawAtDesiredSize(true);
    InternalWidgetComp->SetVisibility(false);

    // 设置默认 Pivot 为底部中心，方便在头顶对齐
    InternalWidgetComp->SetPivot(FVector2D(HorizontalOffset, VerticalOffset));
}


// Called when the game starts
void USpeakInworldComponent::BeginPlay()
{
	Super::BeginPlay();
 //   if(InternalWidgetComp->GetWidgetClass() == nullptr)
 //   {
 //       UE_LOG(LogTemp, Warning, TEXT("SpeakInworldComponent: InternalWidgetComp has no WidgetClass set. Please assign a UserWidget Blueprint to it."));
	//}
}

void USpeakInworldComponent::OnRegister()
{
    Super::OnRegister();

    // 强制 InternalWidgetComp 重新挂载并更新偏移（WidgetRelativeOffset建议仍用UPROPERTY管理）
    if (InternalWidgetComp)
    {
        // 只要不是已挂载的，重新Attach
        if (InternalWidgetComp->GetAttachParent() != this)
        {
            InternalWidgetComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
        }
        InternalWidgetComp->SetMobility(EComponentMobility::Movable);  // 避免父子mobility不匹配
    }
}


// Called every frame
void USpeakInworldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    //始终朝向摄像机
    if (InternalWidgetComp && InternalWidgetComp->IsVisible())
    {
        APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
        if (CamManager)
        {
            FVector WidgetLoc = InternalWidgetComp->GetComponentLocation();
            FVector CamLoc = CamManager->GetCameraLocation();

            FRotator LookAtRot = (CamLoc - WidgetLoc).Rotation();

            InternalWidgetComp->SetWorldRotation(LookAtRot);
        }
    }
}

void USpeakInworldComponent::AddSpeakContentAndSpeak(const FString& Content)
{
    UE_LOG(LogTemp, Log, TEXT("SpeakInworldComponent: Adding content to speak queue: %s"), *Content);
    if (!InternalWidgetComp)
    {
		UE_LOG(LogTemp, Warning, TEXT("SpeakInworldComponent: InternalWidgetComp is null, cannot add speak content."));
        return;
    }

	SpeakContentQueue.Add(Content);

	//如果当前UI没有显示，直接更新UI显示第一条内容
    if(!InternalWidgetComp->IsVisible())
    {
        UpdateSpeakUI();
	}
}

void USpeakInworldComponent::UpdateSpeakUI()
{
    UE_LOG(LogTemp, Log, TEXT("SpeakInworldComponent: UpdateSpeakUI"));
    //如果当前队列中还有内容，继续显示下一条
    if (!SpeakContentQueue.IsEmpty())
    {
        FString NextContent = SpeakContentQueue[0];
		SpeakContentQueue.RemoveAt(0);
        UUserWidget* UserWidget = InternalWidgetComp->GetUserWidgetObject();
        if (UserWidget)
        {
            //通过蓝图中的文本昵称来获取文本组件，并更新文本内容，注意这里需要保证蓝图中有一个叫做“SpeakContent”的文本组件，并且勾选Is Variable
            UTextBlock* TextBlock = Cast<UTextBlock>(UserWidget->GetWidgetFromName(FName(TEXT("SpeakContent"))));
            if (TextBlock)
            {
                TextBlock->SetText(FText::FromString(NextContent));
            }
        }
        else
        {
			UE_LOG(LogTemp, Warning, TEXT("SpeakInworldComponent: UserWidget is null when trying to update speak content."));
        }
        InternalWidgetComp->SetVisibility(true);

		//根据内容长度计算停留时间，基础时间加上每个字符增加的时间
        float Duration = BaseDisplayTime + (NextContent.Len() * TimePerCharacter);
        GetWorld()->GetTimerManager().SetTimer(SpeakTimerHandle, this, &USpeakInworldComponent::UpdateSpeakUI, Duration, false);
    }
    //如果当前队列中没有内容了，隐藏UI
    else
    {
        InternalWidgetComp->SetVisibility(false);
    }
}
