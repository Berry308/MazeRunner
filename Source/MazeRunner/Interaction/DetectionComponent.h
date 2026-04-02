// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DetectionComponent.generated.h"

// 多播委托，通知UI更新当前可交互物体列表
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableListUpdated, const TArray<AActor*>&, Interactables);
// 多播委托，当前选中索引变化通知UI高亮
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedIndexChanged, int32, NewIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAZERUNNER_API UDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDetectionComponent();

    static UDetectionComponent* FindDetectionComponent(AActor* Actor) { return Actor ? Actor->FindComponentByClass<UDetectionComponent>() : nullptr; }

    // 事件委托
    UPROPERTY(BlueprintAssignable, Category = "Interactable Detection")
    FOnInteractableListUpdated OnInteractableListUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Interactable Detection")
    FOnSelectedIndexChanged OnSelectedIndexChanged;

    // 切换选中目标，bForward=true向后，false向前循环
    UFUNCTION(BlueprintCallable, Category = "Interactable Detection")
    void SelectNextInteractable(bool bForward);

    // 获取当前选中目标
    UFUNCTION(BlueprintCallable, Category = "Interactable Detection")
    AActor* GetCurrentInteractable() const;

    // 扫描半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Detection")
    float ScanRadius = 300.f;

    // 扫描间隔(秒)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Detection")
    float ScanInterval = 0.15f;

    // 当前可交互物体列表
    UPROPERTY(BlueprintReadOnly, Category = "Interactable Detection")
    TArray<AActor*> CurrentInteractables;

    // 当前选中索引
    UPROPERTY(BlueprintReadOnly, Category = "Interactable Detection")
    int32 SelectedIndex = 0;

protected:
    virtual void BeginPlay() override;

    // 定时扫描执行函数
    void ScanForInteractables();

    bool CanInteractWith(AActor* SourceActor, AActor* TargetToInteract) const;

private:
    FTimerHandle ScanTimerHandle;
};
