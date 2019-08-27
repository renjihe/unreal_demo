// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityObj.h"
#include "EntityComponent.h"
#include "SpGame.h"
#include "Res/ResLibrary.h"
#include "SprayMgr.h"
#include "Kismet/KismetMathLibrary.h"

#include "client/dllclient.h"

AAbilityObj::AAbilityObj(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		EntityComponent = MakeShareable(new FEntityComponent(this));
	}

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultComponent"));

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	Faction = 0;
}

void AAbilityObj::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	EntityComponent = NULL;
}

void AAbilityObj::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float *pos, *dir;
	if (spclient::getEntityMovement(EntityId, &pos, &dir))
	{
		SetActorLocationAndRotation(FVector(pos[0], pos[1], pos[2])
			, FRotationMatrix::MakeFromXZ(FVector(dir[0], dir[1], dir[2]), FVector(0, 0, 1)).Rotator());
	}
}
