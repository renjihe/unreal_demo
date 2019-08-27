// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "LevelConfigActor.h"


// Sets default values
ALevelConfigActor::ALevelConfigActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ALevelConfigActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelConfigActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

