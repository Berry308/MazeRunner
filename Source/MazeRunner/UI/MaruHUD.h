// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MaruHUD.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class MAZERUNNER_API AMaruHUD : public AHUD
{
	GENERATED_BODY()
	
public:
    // Widget 栈，确保 GC 不会提前销毁 Widget
    UPROPERTY()
    TArray<UUserWidget*> WidgetStack;

    // 推入一个Widget到栈顶（显示并切UI输入）
    UFUNCTION(BlueprintCallable)
    void PushWidget(UUserWidget* NewWidget);

    // 弹出并关闭栈顶Widget，恢复上一个状态
    UFUNCTION(BlueprintCallable)
    void PopWidget();

protected:
    // 根据栈情况切换输入模式
    void RefreshInputMode();
};
