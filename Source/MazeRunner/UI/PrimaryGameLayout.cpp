// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PrimaryGameLayout.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/CanvasPanel.h"
#include "ActivatableWidgetStack.h"
#include "ActivatableWidget.h"
#include "MazeRunnerLogChannels.h"

void UPrimaryGameLayout::NativeConstruct()
{
	Super::NativeConstruct();

    //成员有效性检查
    
    //if(RootCanvas)
    //{
    //	UCanvasPanelSlot* StackSlot = RootCanvas->AddChildToCanvas(HUDLayerStack);
    //
    //	if (StackSlot)
    //	{
    //		// 3. 设置锚点为全屏填充 (0,0 到 1,1)
    //		StackSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    //
    //		// 4. 将偏移清零（确保真正对齐到边缘）
    //		StackSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
    //
    //		// 5. 设置 Z-Order：HUD 为最低层
    //		StackSlot->SetZOrder(0);
    //
    //		// 如果是 MenuStack，可以设置 ZOrder 为 10，以此类推
    //	}
    //}
}

void UPrimaryGameLayout::UpdateCurrentTopWidget()
{
    if (InteractableUILayerStack->GetTopWidget())
    {
		CurrentTopWidget = InteractableUILayerStack->GetTopWidget();
    }
    else if(HUDLayerStack->GetTopWidget())
    {
        CurrentTopWidget = HUDLayerStack->GetTopWidget();
    }
    else
    {
        CurrentTopWidget = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("No Active widget in PrimaryGameLayout!"));
	}
}

void UPrimaryGameLayout::PushWidgetToLayer(EWidgetLayer WidgetLayer, UActivatableWidget* Widget)
{
    switch (WidgetLayer)
    {
         case EWidgetLayer::HUD:
         {
             if (HUDLayerStack)
             {
                 HUDLayerStack->PushWidget(Widget);
                 CurrentTopWidget = Widget;
             }
             break;
         }
         case EWidgetLayer::InteractableUI:
         {
             if (InteractableUILayerStack)
             {
                 InteractableUILayerStack->PushWidget(Widget);
                 CurrentTopWidget = Widget;
             }
             break;
         }
         case EWidgetLayer::TopUI:
         {
             if (TopUILayerStack)
             {
                 TopUILayerStack->PushWidget(Widget);
                 //TopUI层的Widget通常不影响输入模式，所以不更新CurrentTopWidget
             }
             break;
         }
         default:
             break;
	}
	RefreshInputMode();
    UE_LOG(LogUI, Log, TEXT("CurrentTopWidget: %s"), *CurrentTopWidget->GetName());
}

void UPrimaryGameLayout::PopWidgetFromLayer(EWidgetLayer WidgetLayer)
{
    switch (WidgetLayer)
    {
    case EWidgetLayer::HUD:
        if (HUDLayerStack)
        {
            //如果栈顶为空
            if (HUDLayerStack->GetTopWidget() == nullptr) return;
            if (CurrentTopWidget == HUDLayerStack->GetTopWidget())
            {
                CurrentTopWidget = nullptr;
            }
            HUDLayerStack->PopWidget();
        }
        break;
    case EWidgetLayer::InteractableUI:
        if (InteractableUILayerStack)
        {
            if (InteractableUILayerStack->GetTopWidget() == nullptr) return;
            if(CurrentTopWidget == InteractableUILayerStack->GetTopWidget())
            {
                CurrentTopWidget = nullptr;
			}
            InteractableUILayerStack->PopWidget();
        }
        break;
    case EWidgetLayer::TopUI:
        if (TopUILayerStack)
        {
            if (TopUILayerStack->GetTopWidget() == nullptr) return;

            TopUILayerStack->PopWidget();
            //TopUI层的Widget通常不影响输入模式，所以不更新CurrentTopWidget。Problem：不更新CurrentTopWidget的话，TopWidget没有输入焦点，所以一定是即时会自动释放，体验可能比较不好。
        }
        break;
    default:
        break;
    }

	//在更新输入模式之前，先从其他层的栈顶寻找新的CurrentTopWidget
	UpdateCurrentTopWidget();
	RefreshInputMode();
    UE_LOG(LogUI, Log, TEXT("CurrentTopWidget: %s"), *CurrentTopWidget->GetName());
}

void UPrimaryGameLayout::RefreshInputMode()
{
    check(OwningPlayerController);
    if (CurrentTopWidget)
    {
		FWidgetInputModeConfig config = CurrentTopWidget->GetInputModeConfig();
        // 栈中有 UI
        switch (config.InputMode)
        {
            case EWidgetInputMode::GameOnly:
            {
                FInputModeGameOnly InputMode;
                InputMode.SetConsumeCaptureMouseDown(config.bLockMouseToViewport);//在从UI模式切换回游戏模式时，是否吞掉此次点击导致的游戏响应（通常体现在射击游戏中）
                OwningPlayerController->SetInputMode(InputMode);
                OwningPlayerController->bShowMouseCursor = config.bShowMouseCursor;//一般为false
                break;
            }
            case EWidgetInputMode::GameAndUI:
            {
                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(CurrentTopWidget->GetCachedWidget());
                InputMode.SetLockMouseToViewportBehavior(config.bLockMouseToViewport ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock);
                OwningPlayerController->SetInputMode(InputMode);
                OwningPlayerController->bShowMouseCursor = config.bShowMouseCursor;
                break;
            }
            case EWidgetInputMode::UIOnly:
            {
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(CurrentTopWidget->GetCachedWidget());
                InputMode.SetLockMouseToViewportBehavior(config.bLockMouseToViewport ? EMouseLockMode::LockAlways : EMouseLockMode::DoNotLock);
                OwningPlayerController->SetInputMode(InputMode);
                OwningPlayerController->bShowMouseCursor = config.bShowMouseCursor;
                break;
            }
            default:
                break;
        }
    }
    else
    {
        // 栈已清空：消除鼠标显示，重新锁定到游戏
		UE_LOG(LogTemp, Error, TEXT("No Active widget in PrimaryGameLayout! Reverting to GameOnly input mode."));
        FInputModeGameOnly InputMode;
        OwningPlayerController->SetInputMode(InputMode);
		OwningPlayerController->bShowMouseCursor = false;
    }
}
