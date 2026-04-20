// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "AI/AIActionBase.h"
#include "ActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionListEmpty);

USTRUCT(BlueprintType)
struct MAZERUNNER_API FAIActionInfoField
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FString FieldName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FString Description;
};

USTRUCT(BlueprintType)
struct MAZERUNNER_API FAIActionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<FAIActionInfoField> Fields;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TSubclassOf<UAIActionBase> ActionClass;
};

/**
 * 
 */
UCLASS(Meta=(BlueprintSpawnableComponent))
class MAZERUNNER_API UActionComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static UActionComponent* FindActionComponent(const AActor* Actor){return (Actor ? Actor->FindComponentByClass<UActionComponent>() : nullptr);}

	//是否当前Action队列为空并且没有正在执行的Action
	UFUNCTION(BlueprintCallable)
	bool IsExecutingAction() const { return CurrentAction != nullptr; }

	//获取当前可以执行的Action队列
	UFUNCTION(BlueprintCallable)
	const TArray<FAIActionInfo>& GetAllowedActionInfor() const{return ActionInfos;};

	//添加Action到队列中
	UFUNCTION(BlueprintCallable)
	void AddActionToQueue(UAIActionBase* Action);

	UFUNCTION(BlueprintCallable)
	void UpdateActionQueue();

	UFUNCTION()
	void OnCurrentActionFinished();
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnActionListEmpty OnActionListEmpty;

protected:
	UPROPERTY(EditAnywhere)
	TArray<FAIActionInfo> ActionInfos;

	UPROPERTY()
	UAIActionBase* CurrentAction;

	// 可用带优先级的队列维护待执行 Action；未定型前先用 TArray，避免无效的标准库包含与不完整模板形参
	UPROPERTY()
	TArray<UAIActionBase*> ActionList;
};
