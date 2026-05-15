// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MaruHUD.h"
#include "Blueprint/UserWidget.h"
#include "ActivatableWidgetStack.h"
#include "PrimaryGameLayout.h"
#include "MazeRunnerLogChannels.h"


//void AMaruHUD::PushInteractableWidget(UUserWidget* NewWidget)
//{
//    if (!NewWidget) return;
//
//    // 1. 添加到视口渲染
//    NewWidget->AddToViewport();
//
//    // 2. 压入 TArray (等同于 Stack Push)
//    InteractableWidgetStack.Push(NewWidget);
//
//    // 3. 更新输入模式，清除鼠标锁定
//    RefreshInputMode();
//}
//
//void AMaruHUD::PopInteractableWidget()
//{
//    if (InteractableWidgetStack.Num() > 0)
//    {
//        // 1. 获取并移除栈顶元素 (等同于 Stack Pop)
//        UUserWidget* TopWidget = InteractableWidgetStack.Pop();
//
//        if (TopWidget)
//        {
//            // 2. 从屏幕移除
//            TopWidget->RemoveFromParent();
//        }
//
//        // 3. 更新输入模式，如果没有 UI 了则恢复鼠标锁定
//        RefreshInputMode();
//    }
//}
//
//void AMaruHUD::RefreshInputMode()
//{
//    APlayerController* PC = GetOwningPlayerController();
//    if (!PC) return;
//    if (InteractableWidgetStack.Num() > 0)
//    {
//        // 栈中有 UI：解锁鼠标，允许交互
//        FInputModeGameAndUI InputMode;
//        InputMode.SetWidgetToFocus(InteractableWidgetStack.Last()->TakeWidget());
//        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
//
//        PC->SetInputMode(InputMode);
//        PC->bShowMouseCursor = true;
//    }
//    else
//    {
//        // 栈已清空：消除 (Eliminate) 鼠标显示，重新锁定到游戏
//        FInputModeGameOnly InputMode;
//        PC->SetInputMode(InputMode);
//        PC->bShowMouseCursor = false;
//    }
//}

AMaruHUD::AMaruHUD()
{
}

void AMaruHUD::AddWidgetToLayer(EWidgetLayer WidgetLayer, UActivatableWidget* NewWidget)
{
	if(PrimaryGameLayoutInstance)
	{
		PrimaryGameLayoutInstance->PushWidgetToLayer(WidgetLayer,NewWidget);
		UE_LOG(LogUI, Log, TEXT("Widget:%s added to layer %s"), *NewWidget->GetName(), *UEnum::GetValueAsString(WidgetLayer));
	}
	else
	{
		UE_LOG(LogUI, Error, TEXT("AMaruHUD:PrimaryGameLayoutInstance is not initialized when Add %s To %s!"),*NewWidget->GetName(), *UEnum::GetValueAsString(WidgetLayer));
	}
}

void AMaruHUD::RemoveTopWidgetFrom(EWidgetLayer WidgetLayer)
{
	if (PrimaryGameLayoutInstance)
	{
		PrimaryGameLayoutInstance->PopWidgetFromLayer(WidgetLayer);
		UE_LOG(LogUI, Log, TEXT("Top widget removed from layer %s"), *UEnum::GetValueAsString(WidgetLayer));
	}
	else
	{
		UE_LOG(LogUI, Error, TEXT("AMaruHUD:PrimaryGameLayoutInstance is not initialized when RemoveTopWidgetFrom!"));
	}
}

void AMaruHUD::BeginPlay()
{
	if (PrimaryGameLayoutClass)
	{
		UWorld* World = GetWorld();
		APlayerController* PC = GetOwningPlayerController();

		PrimaryGameLayoutInstance = CreateWidget<UPrimaryGameLayout>(PC, PrimaryGameLayoutClass);
		if (PrimaryGameLayoutInstance)
		{
			PrimaryGameLayoutInstance->AddToViewport();
			PrimaryGameLayoutInstance->SetOwningHUD(this);
			PrimaryGameLayoutInstance->SetOwningPlayerController(GetOwningPlayerController());
			UE_LOG(LogUI, Log, TEXT("PrimaryGameLayoutInstance instantiated."));
		}
	}
	else
	{
		UE_LOG(LogUI, Error, TEXT("PrimaryGameLayoutClass is not set in AMaruHUD!"));
	}
	Super::BeginPlay();
}
