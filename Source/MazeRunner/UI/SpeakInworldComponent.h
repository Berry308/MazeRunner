// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpeakInworldComponent.generated.h"

class UWidgetComponent;

/*
* 在游戏中显示角色头顶的对话内容组件，内部持有一个 WidgetComponent 用于渲染 UI，并提供接口添加对话内容。
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAZERUNNER_API USpeakInworldComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpeakInworldComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 添加Speak的内容到列表中，并在游戏中Speak */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddSpeakContentAndSpeak(const FString& Content);

private:
    /** 内部持有的渲染组件 */
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    UWidgetComponent* InternalWidgetComp;

    /** 定时器句柄，用于控制消失逻辑 */
    FTimerHandle SpeakTimerHandle;

    TArray<FString> SpeakContentQueue;

    /** 根据当前SpeakContentQueue是否有内容来更新UI显示 */
    void UpdateSpeakUI();

    /** 基础停留时间 */
    UPROPERTY(EditAnywhere, Category = "Dialogue|Settings")
    float BaseDisplayTime = 1.5f;

    /** 每个字符增加的显示时间 */
    UPROPERTY(EditAnywhere, Category = "Dialogue|Settings")
    float TimePerCharacter = 0.08f;

    UPROPERTY(EditAnywhere, Category = "Dialogue|Settings")
    float HorizontalOffset = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Dialogue|Settings")
	float VerticalOffset = 2.0f;
};
