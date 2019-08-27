// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "BarracksActor.h"
#include "AIControllerSV.h"
#include "AIControllerCrowd.h"
#include "MobCharacter.h"

ABarracksActor::ABarracksActor() 
	: Number(1), Frequency(2), SpawnTeamNum(EMonsterType::Boss), PrevTime(0), AutoCreate(true)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	USphereComponent* component = CreateDefaultSubobject<USphereComponent>(TEXT("Barracks"));
	component->SetupAttachment(RootComponent);
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABarracksActor::BeginPlay()
{
	Super::BeginPlay();
	PrevTime = Frequency;
}

// Called every frame
void ABarracksActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (Role == ROLE_Authority)
	{
		PrevTime += deltaTime;
		if (PrevTime > Frequency && AutoCreate)
		{
			PrevTime = 0.0f;
			CreateZombie();
		}
	}	
}

void ABarracksActor::CreateZombie()
{
	if (Number < 1 || Frequency < 0)
	{
		Number = 1;
		Frequency = 2;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FVector loc = GetActorLocation();
	FRotator rot = GetActorRotation();
	
	for (int i = 1; i <= Number; ++i)
	{
		ACharacterSV* const Mob = GetWorld()->SpawnActor<ACharacterSV>(PawnClass ? PawnClass : ACharacterSV::StaticClass(), loc, rot, SpawnInfo);
		if (Mob)
		{
			Mob->SetLook(LookId);
			Mob->JobId = JobId;
			Mob->TeamID = 1;
			Mob->AIExtra = AIExtra;
			Mob->AIControllerClass = AAIControllerSV::StaticClass();// AIExtra.bBuildSubPath ? AAIControllerSV::StaticClass() : AAIControllerCrowd::StaticClass();
			Mob->SpawnDefaultController();
			Mob->Init(loc, rot);
			//Mob->InitAbility();

			/*AAIControllerSV *AC = Cast<AAIControllerSV>(Mob->GetController());
			if (AC) {
				AC->RunBehaviorTree(0);
			}*/
		}
	}
}