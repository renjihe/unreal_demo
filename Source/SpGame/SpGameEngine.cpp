// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameEngine.h"
#include "client/dllclient.h"
#include <assert.h>

void USpGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	assert((_controlfp(0, 0) & _MCW_PC) == _PC_24);
	assert((_controlfp(0, 0) & _MCW_RC) == _RC_NEAR);

	Super::Tick(DeltaSeconds, bIdleMode);

	spclient::tick(DeltaSeconds);
}



