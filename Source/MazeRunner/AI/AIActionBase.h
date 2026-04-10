// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AIActionBase.generated.h"

class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionFinished);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class MAZERUNNER_API UAIActionBase : public UObject
{
	GENERATED_BODY()

public:
	/** 绑定执行该行为的 Pawn（通常在入队前调用）。 */
	UFUNCTION(BlueprintCallable, Category = "AI|Action")
	void BindOwnerPawn(APawn* InOwner);

	UFUNCTION(BlueprintPure, Category = "AI|Action")
	const APawn* GetOwnerPawn() { return OwnerPawn; }

	UFUNCTION(BlueprintCallable, Category = "AI|Action")
	bool IsExecuting() const { return bIsExecuting; }

	/**
	 * 用解析得到的键值填充行为参数；键应与对应 FAIActionInfo 里声明的 FieldName 一致。
	 * （不使用 UFUNCTION：TMap 不能暴露给蓝图反射。子类在 C++ 中 override。）
	 * @return 参数合法且已就绪则为 true，否则不应入队或应走兜底逻辑。
	 */
	virtual bool ConfigureFromParsedParams(const TMap<FString, FString>& RawParams);

	UFUNCTION(BlueprintNativeEvent, Category = "AI|Action")
	void Execute();
	virtual void Execute_Implementation();

public:
	// 行为优先级，数值越大优先级越高
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 Priority = 0;

	// 行为完成通知委托
	UPROPERTY(BlueprintAssignable)
	FOnActionFinished OnActionFinished;

protected:
	UPROPERTY(BlueprintReadOnly, Transient, Category = "AI|Action")
	APawn* OwnerPawn = nullptr;
private:
	bool bIsExecuting = false;
};
