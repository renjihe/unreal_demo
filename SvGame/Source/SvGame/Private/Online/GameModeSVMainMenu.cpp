// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameModeSVMainMenu.h"
#include "GameSessionSV.h"
#include "HVInterface.h"
#include "PlayerStateSV.h"
#include "MajorCharacter.h"
#include "PlayerControllerSV.h"
#include "HVInterface.h"

AGameModeSVMainMenu::AGameModeSVMainMenu(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{	
	static ConstructorHelpers::FObjectFinder<UBlueprint> charBP(TEXT("Class'/Game/Blueprints/Character/CharacterSelect.CharacterSelect'"));
	if (charBP.Object) {
		DefaultPawnClass = charBP.Object->GeneratedClass;
	}
	else
	{
		DefaultPawnClass = APawn::StaticClass();
	}

	PlayerStateClass = APlayerState::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}

void AGameModeSVMainMenu::BeginPlay()
{
	Super::BeginPlay();
}

void AGameModeSVMainMenu::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}