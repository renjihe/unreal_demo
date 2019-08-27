// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"

#include "AIControllerCrowd.h"

#include "Navigation/CrowdFollowingComponent.h"

AAIControllerCrowd::AAIControllerCrowd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
}
