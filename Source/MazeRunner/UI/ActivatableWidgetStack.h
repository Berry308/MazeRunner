// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Overlay.h"
#include "ActivatableWidget.h"
#include "ActivatableWidgetStack.generated.h"

struct FWidgetInputModeConfig;
/**
 * 用来存储ActivatableWidget的栈，管理它们的显示
 */
UCLASS()
class MAZERUNNER_API UActivatableWidgetStack : public UOverlay
{
	GENERATED_BODY()

public:
	UActivatableWidget* GetTopWidget(){ return WidgetStack.Num() > 0 ? WidgetStack.Top() : nullptr; }

	// 推入一个Widget到栈顶，并更新InputMode
	UFUNCTION(BlueprintCallable)
	void PushWidget(UActivatableWidget* NewWidget);

	// 弹出并关闭栈顶Widget，如果栈顶还有Widget，恢复它的显示
	UFUNCTION(BlueprintCallable)
	void PopWidget();
	
protected:
	// 根据栈情况切换输入模式
	//void RefreshInputMode(FWidgetInputModeConfig config);

private:


	//Question：要不要设计两个栈？用于区分可交互和不可交互的UI，以便更细粒度地控制输入模式和鼠标锁定行为？不需要，复杂化了
	UPROPERTY()
	TArray<UActivatableWidget*> WidgetStack;
};
