// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tasks/AITask.h"
#include "AITask_WaitDelay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAIWaitDelayDelegate);
/**
 * 
 */
UCLASS()
class SVGAME_API UAITask_WaitDelay : public UAITask
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FAIWaitDelayDelegate	OnFinish;

	virtual void Activate() override;

	/** Return debug string describing task */
	virtual FString GetDebugString() const override;

	/** Wait specified time. This is functionally the same as a standard Delay node. */
	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Tasks", meta = (DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE"))
	static UAITask_WaitDelay* WaitDelay(AAIController* Controller, APawn* Pawn, float Time);

private:
	FTimerHandle TimerHandle;
	void OnTimeFinish();

	virtual void OnDestroy(bool AbilityEnded) override;
	float Time;
	float TimeStarted;
};
