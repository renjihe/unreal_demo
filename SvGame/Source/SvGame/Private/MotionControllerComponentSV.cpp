// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "MotionControllerComponentSV.h"
#include <UnrealNetwork.h>

UMotionControllerComponentSV::UMotionControllerComponentSV(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	bReplicates = true;
}

void UMotionControllerComponentSV::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CHANGE_CONDITION(UMotionControllerComponentSV, RelativeLocation, COND_SkipOwner);
	DOREPLIFETIME_CHANGE_CONDITION(UMotionControllerComponentSV, RelativeRotation, COND_SkipOwner);
}


