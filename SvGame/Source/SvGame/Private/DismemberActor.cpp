// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "DismemberActor.h"


// Sets default values
ADismemberActor::ADismemberActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));	
	RootComponent = Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
}

// Called when the game starts or when spawned
void ADismemberActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADismemberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

