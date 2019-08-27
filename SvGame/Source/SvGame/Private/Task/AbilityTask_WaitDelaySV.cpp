// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "SvGame.h"

#include "AbilityTask_WaitDelaySV.h"
#include "TimerManager.h"
#include "AbilitySystemGlobals.h"

UAbilityTask_WaitDelaySV::UAbilityTask_WaitDelaySV(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Time = 0.f;
	TimeStarted = 0.f;
}

UAbilityTask_WaitDelaySV* UAbilityTask_WaitDelaySV::WaitDelay(UGameplayAbility* OwningAbility, float Time)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Duration(Time);

	auto MyObj = NewAbilityTask<UAbilityTask_WaitDelaySV>(OwningAbility);
	MyObj->Time = Time;
	return MyObj;
}

void UAbilityTask_WaitDelaySV::Activate()
{
	UWorld* World = GetWorld();
	TimeStarted = World->GetTimeSeconds();

	// Use a dummy timer handle as we don't need to store it for later but we don't need to look for something to clear
	World->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_WaitDelaySV::OnTimeFinish, Time, false);
}

void UAbilityTask_WaitDelaySV::OnDestroy(bool AbilityEnded)
{
	Super::OnDestroy(AbilityEnded);

	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(TimerHandle);
}

void UAbilityTask_WaitDelaySV::OnTimeFinish()
{
	OnFinish.Broadcast();
	EndTask();
}

FString UAbilityTask_WaitDelaySV::GetDebugString() const
{
	float TimeLeft = Time - GetWorld()->TimeSince(TimeStarted);
	return FString::Printf(TEXT("WaitDelay. Time: %.2f. TimeLeft: %.2f"), Time, TimeLeft);
}