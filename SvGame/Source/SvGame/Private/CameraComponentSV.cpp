// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "CameraComponentSV.h"
#include <UnrealNetwork.h>

UCameraComponentSV::UCameraComponentSV(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	bReplicates = true;
}

void UCameraComponentSV::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CHANGE_CONDITION(UCameraComponentSV, RelativeLocation, COND_SkipOwner);
	DOREPLIFETIME_CHANGE_CONDITION(UCameraComponentSV, RelativeRotation, COND_SkipOwner);
}

