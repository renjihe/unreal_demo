// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "SvGame.h"
#include "Task/AITask_ActivateAbility.h"
#include "AIController.h"
#include "VisualLogger/VisualLogger.h"
#include "GameplayTasksComponent.h"

UAITask_ActivateAbility::UAITask_ActivateAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAITask_ActivateAbility* UAITask_ActivateAbility::ActivateAbility(AAIController* Controller, APawn* Pawn, FVector GoalLocation, AActor* GoalActor, int AbilityId, bool bLockAILogic)
{
	UAITask_ActivateAbility* MyTask = Controller && Pawn ? UAITask::NewAITask<UAITask_ActivateAbility>(*Controller, EAITaskPriority::High) : nullptr;
	if (MyTask)
	{
		MyTask->AddRequiredResource(UAIResource_ActivateAbility::StaticClass());
		MyTask->AddClaimedResource(UAIResource_ActivateAbility::StaticClass());

		MyTask->AbilityReq.Pawn = Pawn;
		MyTask->AbilityReq.GoalActor = GoalActor;
		MyTask->AbilityReq.GoalLocation = GoalLocation;
		MyTask->AbilityReq.AbilityId = AbilityId;
		MyTask->SetUp(Controller);
		
		if (bLockAILogic) {
			MyTask->RequestAILogicLocking();
		}
	}

	return MyTask;
}

void UAITask_ActivateAbility::OnRequestFinishedCallBack(int AbilityId)
{
	FinishMoveTask(EAbilityTaskResult::Finished, AbilityId);
}

void UAITask_ActivateAbility::OnRequestSuccessCallBack(int AbilityId)
{
	OnRequestSuccess.Broadcast();
}

void UAITask_ActivateAbility::SetUp(AAIController* Controller)
{
	OwnerController = Controller;
}

void UAITask_ActivateAbility::SetUpCallBack(bool bBind)
{
	ACharacterSV *Pawn = Cast<ACharacterSV>(AbilityReq.Pawn);
	if (Pawn)
	{
		if (bBind)
		{
			if (RequestFinishedDelegateHandle.IsValid())
			{
				Pawn->OnActivateAbilityFinished.Remove(RequestFinishedDelegateHandle);
				RequestFinishedDelegateHandle.Reset();
			}
			if (RequestSuccessDelegateHandle.IsValid())
			{
				Pawn->OnActivateAbilitySuccess.Remove(RequestSuccessDelegateHandle);
				RequestSuccessDelegateHandle.Reset();
			}
		}
		else
		{
			RequestFinishedDelegateHandle = Pawn->OnActivateAbilityFinished.AddUObject(this, &UAITask_ActivateAbility::OnRequestFinishedCallBack);
			RequestSuccessDelegateHandle = Pawn->OnActivateAbilitySuccess.AddUObject(this, &UAITask_ActivateAbility::OnRequestSuccessCallBack);
		}
	}
}

void UAITask_ActivateAbility::Activate()
{
	Super::Activate();

	ACharacterSV *Pawn = Cast<ACharacterSV>(AbilityReq.Pawn);
	if (Pawn)
	{
		SetUpCallBack(false);

		if (!Pawn->TryActivateAbilityByID(AbilityReq.AbilityId)) {
			FinishMoveTask(EAbilityTaskResult::Failed);
		}
	}
	else
	{
		FinishMoveTask(EAbilityTaskResult::Failed);
	}
}

void UAITask_ActivateAbility::Pause()
{
	Super::Pause();
	
	SetUpCallBack(true);
}

void UAITask_ActivateAbility::Resume()
{
	Super::Resume();

	SetUpCallBack(false);
}

void UAITask_ActivateAbility::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

void UAITask_ActivateAbility::FinishMoveTask(EAbilityTaskResult::Type InResult, int AbilityId)
{
	SetUpCallBack(true);

	EndTask();

	if (InResult == EAbilityTaskResult::Failed)
	{
		OnRequestFailed.Broadcast();
	}
	else
	{
		OnRequestFinished.Broadcast(AbilityId, OwnerController);
	}
}
