// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActivatableWidget.generated.h"

UENUM(BlueprintType)
enum class EWidgetInputMode : uint8
{
    GameAndUI,     
    UIOnly,        
    GameOnly       
};

USTRUCT(BlueprintType)
struct FWidgetInputModeConfig
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EWidgetInputMode InputMode = EWidgetInputMode::GameAndUI;

    //UPROPERTY(EditAnywhere)
    //EMouseCaptureMode MouseCaptureMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bLockMouseToViewport = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bShowMouseCursor = false;
};
/**
 * 用于ActivatableWidgetStack的Widget基类
 * 提供输入配置，在ActivatableWidgetStack中会根据栈顶Widget的输入配置来设置输入模式和鼠标锁定状态
 */
UCLASS()
class MAZERUNNER_API UActivatableWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    FWidgetInputModeConfig GetInputModeConfig() const;
public:
    // 默认设置为填充或中心对齐
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TEnumAsByte<EVerticalAlignment> VerticalAlignment = VAlign_Fill;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FWidgetInputModeConfig InputModeConfig;
};
