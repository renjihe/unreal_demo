// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AITask_WaitDelay.h"

#include "TimerManager.h"

UAITask_WaitDelay::UAITask_WaitDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Time = 0.f;
	TimeStarted = 0.f;
}

UAITask_WaitDelay* UAITask_WaitDelay::WaitDelay(AAIController* Controller, APawn* Pawn, float Time)
{
	UAITask_WaitDelay* MyTask = Controller && Pawn ? UAITask::NewAITask<UAITask_WaitDelay>(*Controller, EAITaskPriority::High) : nullptr;
	if (MyTask) {
		MyTask->Time = Time;
	}

	return MyTask;
}

void UAITask_WaitDelay::Activate()
{
	UWorld* World = GetWorld();
	TimeStarted = World->GetTimeSeconds();

	// Use a dummy timer handle as we don't need to store it for later but we don't need to look for something to clear
	World->GetTimerManager().SetTimer(TimerHandle, this, &UAITask_WaitDelay::OnTimeFinish, Time, false);
}

void UAITask_WaitDelay::OnDestroy(bool AbilityEnded)
{
	Super::OnDestroy(AbilityEnded);

	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(TimerHandle);
}

void UAITask_WaitDelay::OnTimeFinish()
{
	OnFinish.Broadcast();
	EndTask();
}

FString UAITask_WaitDelay::GetDebugString() const
{
	float TimeLeft = Time - GetWorld()->TimeSince(TimeStarted);
	return FString::Printf(TEXT("WaitDelay. Time: %.2f. TimeLeft: %.2f"), Time, TimeLeft);
}


