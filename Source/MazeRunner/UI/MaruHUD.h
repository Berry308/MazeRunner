// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MaruHUD.generated.h"

class UPrimaryGameLayout;
class UUserWidget;
class UActivatableWidget;
struct FGameplayTag;
enum class EWidgetLayer : uint8;
/**
 * 管理玩家的UI，维护多个栈：窗口栈等
 */
UCLASS()
class MAZERUNNER_API AMaruHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AMaruHUD();

	UFUNCTION(BlueprintCallable)
	void AddWidgetToLayer(EWidgetLayer WidgetLayer, UActivatableWidget* NewWidget);

	//弹出最顶层的可交互Widget
    UFUNCTION(BlueprintCallable)
    void RemoveTopWidgetFrom(EWidgetLayer WidgetLayer);

    // Widget 栈，确保 GC 不会提前销毁 Widget
    //UPROPERTY()
    //TArray<UUserWidget*> InteractableWidgetStack;

    // 推入一个Widget到栈顶（显示并切UI输入）
    //UFUNCTION(BlueprintCallable)
    //void PushInteractableWidget(UUserWidget* NewWidget);

    // 弹出并关闭栈顶Widget，恢复上一个状态
    //UFUNCTION(BlueprintCallable)
    //void PopInteractableWidget();
protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPrimaryGameLayout> PrimaryGameLayoutClass;

protected:
    UPROPERTY()
	UPrimaryGameLayout* PrimaryGameLayoutInstance;
};
