// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MaruHUD.h"
#include "Blueprint/UserWidget.h"

void AMaruHUD::PushWidget(UUserWidget* NewWidget)
{
    if (!NewWidget) return;

    // 1. 添加到视口渲染
    NewWidget->AddToViewport();

    // 2. 压入 TArray (等同于 Stack Push)
    WidgetStack.Push(NewWidget);

    // 3. 更新输入模式，消除 (Eliminate) 鼠标锁定
    RefreshInputMode();
}

void AMaruHUD::PopWidget()
{
    if (WidgetStack.Num() > 0)
    {
        // 1. 获取并移除栈顶元素 (等同于 Stack Pop)
        UUserWidget* TopWidget = WidgetStack.Pop();

        if (TopWidget)
        {
            // 2. 从屏幕移除
            TopWidget->RemoveFromParent();
        }

        // 3. 更新输入模式，如果没有 UI 了则恢复鼠标锁定
        RefreshInputMode();
    }
}

void AMaruHUD::RefreshInputMode()
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;
    if (WidgetStack.Num() > 0)
    {
        // 栈中有 UI：解锁鼠标，允许交互
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(WidgetStack.Last()->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    else
    {
        // 栈已清空：消除 (Eliminate) 鼠标显示，重新锁定到游戏
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}