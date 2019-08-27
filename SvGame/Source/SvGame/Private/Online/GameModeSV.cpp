// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameModeSV.h"
#include "GameSessionSV.h"
#include "HVInterface.h"
#include "PlayerStateSV.h"
#include "MajorCharacter.h"
#include "PlayerControllerSV.h"

AGameModeSV::AGameModeSV(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UBlueprint> charBP(TEXT("Class'/Game/Blueprints/Character/player.player'"));
	if (charBP.Object) {
		DefaultPawnClass = charBP.Object->GeneratedClass;
	}
	else
	{
		DefaultPawnClass = AMajorCharacter::StaticClass();
	}

	PlayerStateClass = APlayerStateSV::StaticClass();
	PlayerControllerClass = APlayerControllerSV::StaticClass();

	//MaxDecal = 30;
}

TSubclassOf<AGameSession> AGameModeSV::GetGameSessionClass() const
{
	return AGameSessionSV::StaticClass();
}

void AGameModeSV::BeginPlay()
{
	Super::BeginPlay();

	//DecalArr.Reset();
}

void AGameModeSV::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*float time = GetWorld()->GetTimeSeconds();
	for (int i = 0; i < DecalArr.Num(); ++i)
	{
		if (DecalArr[i].AutoDestroyTime > time && IsValid(DecalArr[i].Comp)) {
			DecalArr[i].Comp->DestroyComponent();
		}
	}
	DecalArr.Reset();
	*/
	Super::EndPlay(EndPlayReason);
}

void AGameModeSV::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	APlayerControllerSV *pc = Cast<APlayerControllerSV>(newPlayer);

	// update spectator location for client
	//if (NewPC && NewPC->GetPawn() == NULL)
	//{
	//	NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	//}

	if (pc)
	{
		pc->ClientGameStarted();
		pc->ClientStartOnlineGame();
	}
}

void AGameModeSV::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// notify players
	for (FConstControllerIterator it = GetWorld()->GetControllerIterator(); it; ++it)
	{
		APlayerControllerSV *pc = Cast<APlayerControllerSV>(*it);
		if (pc) {
			pc->ClientGameStarted();
		}
	}
}

void AGameModeSV::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
}

void AGameModeSV::ServerTestCmd(const FString &Cmd, const FString &Arg)
{
	const TCHAR *CharArg = *Arg;
	if (Cmd == "SeamLess")
	{
		TCHAR TokenStr[128];

		if (FParse::Token(CharArg, TokenStr, ARRAY_COUNT(TokenStr), true))
		{
			FString url = FString::Printf(TEXT("/Game/Levels/%s?game=%s%s%s?%s=%d%s"), TokenStr, TEXT("FFA"), TEXT("?listen"), TEXT("?bIsLanMatch"), TEXT("Bots"), 4, TEXT("?DemoRec"));
			BeforeSeamlessTravel();
			GetWorld()->ServerTravel(url);
		}
	}
}
/*
void AGameModeSV::AddDecalItem(FDecalItem decal)
{
	DecalArr.Add(decal);

	if (DecalArr.Num() >= MaxDecal)
	{
		float time = GetWorld()->GetTimeSeconds();
		if (DecalArr[0].AutoDestroyTime > time && ::IsValid(DecalArr[0].Comp)) {
			DecalArr[0].Comp->DestroyComponent();
		}
		DecalArr.RemoveAt(0);
	}
}*/