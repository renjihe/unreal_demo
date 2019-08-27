// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "PlayerControllerSV.h"
#include "PlayerStateSV.h"
#include "GameModeSV.h"
#include <OnlineSubsystemUtils.h>


APlayerControllerSV::APlayerControllerSV(const FObjectInitializer& ObjectInitialize)
	: Super(ObjectInitialize)
{
	SayTimer = SayInterval = 0;
}

void APlayerControllerSV::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (Role == ROLE_AutonomousProxy)
	{
		SayTimer += DeltaTime;
		if (SayInterval <= 0.001f)
		{
			//FMath::SRandInit(FPlatformTime::Cycles());		
			SayInterval = FMath::SRand() * 10.0f + 1.0f;
		}
		if (SayTimer > SayInterval)
		{
			TArray<FStringFormatArg> arg;
			arg.Push(SayInterval);
			ServerSay(FString::Format(TEXT("Content({0})"), arg));

			SayInterval = SayTimer = 0;
		}
	}
}

void APlayerControllerSV::ClientGameStarted_Implementation()
{
	// Enable controls mode now the game has started
	SetIgnoreMoveInput(false);

	// Send round start event
	auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			// Generate a new session id
			Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

			// Fire session start event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("MapName"), FVariantData(MapName));

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

			// Online matches require the MultiplayerRoundStart event as well
			//if (SGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				// @todo: fill in with real values
				MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
				MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
			}

			//bHasSentStartEvents = true;
		}
	}
}

/** Starts the online game using the session name in the PlayerState */
void APlayerControllerSV::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	APlayerStateSV *playerState = Cast<APlayerStateSV>(PlayerState);
	if (playerState)
	{
		IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
		if (sessionInterface.IsValid())
		{
			UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *playerState->SessionName.ToString());
			sessionInterface->StartSession(playerState->SessionName);
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &APlayerControllerSV::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void APlayerControllerSV::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	APlayerStateSV *playerState = Cast<APlayerStateSV>(PlayerState);
	if (playerState)
	{
		IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
		if (sessionInterface.IsValid())
		{
			UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *playerState->SessionName.ToString());
			sessionInterface->EndSession(playerState->SessionName);
		}
	}
}

void APlayerControllerSV::ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner)
{
	Super::ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

	/*
	// Disable controls now the game has ended
	SetIgnoreMoveInput(true);

	bAllowGameActions = false;

	// Make sure that we still have valid view target
	SetViewTarget(GetPawn());

	AShooterHUD* ShooterHUD = GetShooterHUD();
	if (ShooterHUD)
	{
		ShooterHUD->SetMatchState(bIsWinner ? EShooterMatchState::Won : EShooterMatchState::Lost);
	}

	UpdateSaveFileOnGameEnd(bIsWinner);
	UpdateAchievementsOnGameEnd();
	UpdateLeaderboardsOnGameEnd();

	// Flag that the game has just ended (if it's ended due to host loss we want to wait for ClientReturnToMainMenu_Implementation first, incase we don't want to process)
	bGameEndedFrame = true;
	*/
}

bool APlayerControllerSV::ServerSay_Validate(const FString &content)
{
	return true;
}

void APlayerControllerSV::ServerSay_Implementation(const FString &content)
{
	NetMulticastSay(content);
}

void APlayerControllerSV::NetMulticastSay_Implementation(const FString &content)
{
	TArray<FStringFormatArg> arg;
	arg.Push(GetUniqueID());
	arg.Push(content);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Format(TEXT("AOmniBaseCharacter_SV::ClientSay_Implementation this={0}, content={1}"), arg));
}

bool APlayerControllerSV::ServerReplicateTrans_Validate(AActor *actor)
{
	return true;
}

void APlayerControllerSV::ServerReplicateTrans_Implementation(AActor *actor)
{
	NetMulticastReplicateTrans(actor, actor->GetRootComponent()->RelativeLocation, actor->GetRootComponent()->RelativeRotation);
}

void APlayerControllerSV::NetMulticastReplicateTrans_Implementation(AActor *actor, const FVector &relativeLocation, const FRotator &relativeRotation)
{
	actor->GetRootComponent()->SetRelativeLocationAndRotation(relativeLocation, relativeRotation);
}

bool APlayerControllerSV::ServerTestCmd_Validate(const FString &Cmd, const FString &Arg)
{
	return true;
}

void APlayerControllerSV::ServerTestCmd_Implementation(const FString &Cmd, const FString &Arg)
{
	AGameModeSV *GameMode = Cast<AGameModeSV>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		GameMode->ServerTestCmd(Cmd, Arg);
	}
}

void APlayerControllerSV::Destroyed()
{
	AGameModeSV *GameMode = Cast<AGameModeSV>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		GameMode->PreLogout(this);
	}

	Super::Destroyed();
}