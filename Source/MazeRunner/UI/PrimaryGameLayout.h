// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PrimaryGameLayout.generated.h"

UENUM(BlueprintType)
enum class EWidgetLayer : uint8
{
	HUD,
	InteractableUI,
	TopUI
};

class UCanvasPanel;
class UOverlay;
class UActivatableWidget;
class UActivatableWidgetStack;
struct FWidgetInputModeConfig;
/**
 * UI的主布局，维护多个Widget栈
 * 被HUD所持有
 */
UCLASS()
class MAZERUNNER_API UPrimaryGameLayout : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void PushWidgetToLayer(EWidgetLayer WidgetLayer, UActivatableWidget* Widget);
	//弹出对应Layer的最顶层Widget，并更新CurrentTopWidget和输入模式
	UFUNCTION()
	void PopWidgetFromLayer(EWidgetLayer WidgetLayer);

	void SetOwningHUD(AHUD* NewOwningHUD) { OwningHUD = NewOwningHUD; }
	void SetOwningPlayerController(APlayerController* NewOwningPlayerController) { OwningPlayerController = NewOwningPlayerController; }

protected:
	virtual void NativeConstruct() override;//在AddToViewport之后调用。重写后，在执行父类逻辑的基础上，添加对各个栈的初始化

	void UpdateCurrentTopWidget();
	//切换输入模式为CurrentTopWidget成员的输入配置
	void RefreshInputMode();


protected:
	UPROPERTY(meta = (BindWidget))
	UOverlay* RootOverlay;
	//UPROPERTY(meta = (BindWidget))
	//UCanvasPanel* RootCanvas;

	//当前最顶层的Widget，决定输入模式。注：不包括TopUILayerStack的Widget，因为它们通常不影响输入模式
	UPROPERTY()
	UActivatableWidget* CurrentTopWidget;

	//基础的在游玩时的UI界面，包含了游戏中常驻的UI元素
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UActivatableWidgetStack* HUDLayerStack;
	//有些窗口需要GameAndUI输入模式，有的需要UIOnly输入模式，如果设计了两个栈，如何控制Z-Order，可以指定相同的值吗？不行。
	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
	UActivatableWidgetStack* InteractableUILayerStack;
	//最顶层的提示UI，通常是一些临时的提示，不影响当前输入模式
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UActivatableWidgetStack* TopUILayerStack;

private:
	UPROPERTY()
	AHUD* OwningHUD;

	UPROPERTY()
	APlayerController* OwningPlayerController;
};
