// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ActivatableWidgetStack.h"
#include "ActivatableWidget.h"
#include "MazeRunnerLogChannels.h"
#include "Components/OverlaySlot.h"

void UActivatableWidgetStack::PushWidget(UActivatableWidget* NewWidget)
{
    if (!NewWidget) return;
	//设置栈顶Widget不可见（如果有的话），以免它仍然响应输入事件
    if (WidgetStack.Num() > 0)
    {
        WidgetStack.Top()->SetVisibility(ESlateVisibility::Collapsed);
    }

    //根据 NewWidget 的配置设置对齐方式
    UOverlaySlot* NewSlot = Cast<UOverlaySlot>(AddChildToOverlay(NewWidget));
    if (NewSlot)
    {
        NewSlot->SetHorizontalAlignment(NewWidget->HorizontalAlignment);
        NewSlot->SetVerticalAlignment(NewWidget->VerticalAlignment);
    }

    WidgetStack.Push(NewWidget);
	NewWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);//Problem：在此处或许要使用ESlateVisibility::SelfHitTestInvisible。防止Visible拦截点击事件，导致无法点击 Widget 内部的按钮。
}

void UActivatableWidgetStack::PopWidget()
{
    if (WidgetStack.Num() > 0)
    {
        //获取并移除栈顶元素
        UUserWidget* TopWidget = WidgetStack.Pop();

        if (TopWidget)
        {
            //从屏幕移除
            TopWidget->RemoveFromParent();
        }
    }

	//如果栈顶还有UI，恢复它的显示
    if (WidgetStack.Num() > 0)
    {
        WidgetStack.Top()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

//有时候一些提示UI，可能需要栈不修改输入模式，或许需要一个bool值来控制
//void UActivatableWidgetStack::RefreshInputMode(FWidgetInputModeConfig config)
//{
//    check(CachedPlayerController);
//    if (WidgetStack.Num() > 0)
//    {
//        // 栈中有 UI
//        switch (config.InputMode)
//        {
//            case EWidgetInputMode::GameOnly:
//                {
//                    FInputModeGameOnly InputMode;
//                    InputMode.SetConsumeCaptureMouseDown(config.bLockMouseToViewport);//在从UI模式切换回游戏模式时，是否吞掉此次点击导致的游戏响应（通常体现在射击游戏中）
//                    CachedPlayerController->SetInputMode(InputMode);
//                    CachedPlayerController->bShowMouseCursor = config.bShowMouseCursor;//一般为false
//                }
//                break;
//            case EWidgetInputMode::GameAndUI:
//                {
//                    FInputModeGameAndUI InputMode;
//                    InputMode.SetWidgetToFocus(WidgetStack.Last()->TakeWidget());
//                    InputMode.SetLockMouseToViewportBehavior(config.bLockMouseToViewport ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock);
//                    CachedPlayerController->SetInputMode(InputMode);
//                    CachedPlayerController->bShowMouseCursor = config.bShowMouseCursor;
//                }
//                break;
//            case EWidgetInputMode::UIOnly:
//                {
//                    FInputModeUIOnly InputMode;
//                    InputMode.SetWidgetToFocus(WidgetStack.Last()->TakeWidget());
//                    InputMode.SetLockMouseToViewportBehavior(config.bLockMouseToViewport ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock);
//                    CachedPlayerController->SetInputMode(InputMode);
//                    CachedPlayerController->bShowMouseCursor = config.bShowMouseCursor;
//                }
//                break;
//			default:
//        }
//    }
//    else
//    {
//        // 栈已清空：消除鼠标显示，重新锁定到游戏
//        FInputModeGameOnly InputMode;
//        CachedPlayerController->SetInputMode(InputMode);
//        CachedPlayerController->bShowMouseCursor = false;
//    }
//}
