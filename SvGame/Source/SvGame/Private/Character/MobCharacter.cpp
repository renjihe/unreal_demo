// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "MobCharacter.h"
#include "AssetData.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Kismet/KismetSystemLibrary.h"


AMobCharacter::AMobCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
	// Ability trace skeleton body only, not Capsule, channel defined in DefaultEngine.ini 
	// [/ Script / Engine.CollisionProfile]
	// +DefaultChannelResponses = (Channel = ECC_GameTraceChannel2, Name = "FireTrace", DefaultResponse = ECR_Block, bTraceType = True, bStaticObject = False)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Ignore);
	TeamID = 1;
	CharRole = ECharacterRole::MOB;
}

void AMobCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
}

void AMobCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	
}

void AMobCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void AMobCharacter::NotifyDead(const FHitResult &hit, EAttackType::Type attackType)
{
	Super::NotifyDead(hit, attackType);

	FTimerHandle timer;
	if (GWorld) {
		GWorld->GetTimerManager().SetTimer(timer, this, &AMobCharacter::DeadAction, 3, false);
	}
}

void AMobCharacter::DeadAction()
{
	for (int i = 0; i < Dismembers.Num(); ++i) {
		if (Dismembers[i]) {
			Dismembers[i]->Destroy();
		}
	}

	if (Controller && !Controller->IsPlayerController()) {
		Controller->Destroy();
	}

	Destroy();
}

