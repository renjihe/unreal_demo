// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tasks/AITask_MoveTo.h"
#include "AITask_MoveToSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API UAITask_MoveToSV : public UAITask_MoveTo
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Tasks", meta = (AdvancedDisplay = "AcceptanceRadius,StopOnOverlap,AcceptPartialPath,bUsePathfinding", DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE", DisplayName = "Move To Location or Actor"))
	static UAITask_MoveTo* AIMoveToWithSpeedType(AAIController* Controller, FVector GoalLocation, AActor* GoalActor = nullptr, float AcceptanceRadius = -1.f, EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default, EAIOptionFlag::Type AcceptPartialPath = EAIOptionFlag::Default, bool bUsePathfinding = true, bool bLockAILogic = true, ESpeedType::Type SpeedType = ESpeedType::None);
};
