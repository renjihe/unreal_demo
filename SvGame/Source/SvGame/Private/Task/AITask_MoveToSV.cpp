// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AITask_MoveToSV.h"


UAITask_MoveTo* UAITask_MoveToSV::AIMoveToWithSpeedType(AAIController* Controller, FVector InGoalLocation, AActor* InGoalActor, float AcceptanceRadius, EAIOptionFlag::Type StopOnOverlap, EAIOptionFlag::Type AcceptPartialPath, bool bUsePathfinding, bool bLockAILogic, ESpeedType::Type SpeedType)
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(Controller);
	if (controller) {
		controller->AISpeedType = SpeedType;
	}

	return UAITask_MoveTo::AIMoveTo(Controller, InGoalLocation, InGoalActor, AcceptanceRadius, StopOnOverlap, AcceptPartialPath, bUsePathfinding, bLockAILogic);
}

