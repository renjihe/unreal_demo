// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameSessionSV.h"
#include "GameInstanceSV.h"
#include "SVBattleMainMenu.h"
#include <OnlineSubsystem.h>
#include <OnlineSubsystemUtils.h>
#include <OnlineSubsystemSessionSettings.h>

#define CustomMatchKeyword FString("MatchKeySV")

/*
* FOnlineSessionSettings implements
*/
FOnlineSessionSettingsSV::FOnlineSessionSettingsSV(bool isLan, bool isPresence, int32 MaxNumPlayers)
	: FOnlineSessionSettings()
{
	NumPublicConnections = MaxNumPlayers;
	if (NumPublicConnections < 0)
	{
		NumPublicConnections = 0;
	}
	NumPrivateConnections = 0;
	bIsLANMatch = isLan;
	bShouldAdvertise = true;
	bAllowJoinInProgress = true;
	bAllowInvites = true;
	bUsesPresence = isPresence;
	bAllowJoinViaPresence = true;
	bAllowJoinViaPresenceFriendsOnly = false;
}

FOnlineSessionSearchSV::FOnlineSessionSearchSV(bool bSearchingLAN, bool bSearchingPresence)
	: FOnlineSessionSearch()
{
	bIsLanQuery = bSearchingLAN;
	MaxSearchResults = 10;
	PingBucketSize = 50;

	if (bSearchingPresence)	{
		QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	}
}

FOnlineSessionSearchEmptyDedicatedSV::FOnlineSessionSearchEmptyDedicatedSV(bool bSearchingLAN, bool bSearchingPresence)
	: FOnlineSessionSearchSV()
{
	QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);
}

/*
* AGameSeesionSV implements
*/
AGameSessionSV::AGameSessionSV(const FObjectInitializer& objectInitializer)
	: AGameSession(objectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnCreateSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnDestroySessionComplete);

		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnJoinSessionComplete);

		OnRegisterPlayerCompleteDelegate = FOnRegisterPlayersCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnRegisterPlayerComplete);

		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &AGameSessionSV::OnStartOnlineGameComplete);
	}	
}

bool AGameSessionSV::HostSession(TSharedPtr<const FUniqueNetId> userId, FName inSessionName, const FString& gameType, const FString& mapName, bool isLan, bool isPresence, int32 maxNumPlayers)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		SessionParams.SessionName = inSessionName;
		SessionParams.IsLAN = isLan;
		SessionParams.IsPresence = isPresence;
		SessionParams.UserId = userId;
		MaxPlayers = maxNumPlayers;

		if (SessionParams.UserId.IsValid())
		{
			HostSettings = MakeShareable(new FOnlineSessionSettingsSV(isLan, isPresence, MaxPlayers));
			HostSettings->Set(SETTING_GAMEMODE, gameType, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MAPNAME, mapName, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			OnStartSessionCompleteDelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			OnRegisterPlayerCompleteDelegateHandle = sessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(OnRegisterPlayerCompleteDelegate);
			return sessionInterface->CreateSession(*SessionParams.UserId, SessionParams.SessionName, *HostSettings);
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		CreatePresenceSessionCompleteEvent.Broadcast(GameSessionName, true);
		return true;
	}
#endif

	return false;
}

void AGameSessionSV::FindSessions(TSharedPtr<const FUniqueNetId> userId, FName sessionName, bool isLan, bool isPresence)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		CurrentSessionParams.SessionName = sessionName;
		CurrentSessionParams.UserId = userId;
		CurrentSessionParams.IsLAN = isLan;
		CurrentSessionParams.IsPresence = isPresence;
		
		if (userId.IsValid())
		{
			SearchSettings = MakeShareable(new FOnlineSessionSearchSV(isLan, isPresence));
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineComparisonOp::Equals);

			OnFindSessionsCompleteDelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();
			sessionInterface->FindSessions(*userId, SearchSettingsRef);
			//Sessions->FindSessions(0, SearchSettingsRef);
		}
	}
}

void AGameSessionSV::OnCreateSessionComplete(FName sessionName, bool isSuccess)
{
	//CurrSessionName = sessionName;

	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	CreatePresenceSessionCompleteEvent.Broadcast(sessionName, isSuccess);
}

void AGameSessionSV::OnDestroySessionComplete(FName sessionName, bool isSuccess)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		HostSettings = NULL;
	}
}

void AGameSessionSV::OnFindSessionsComplete(bool isSuccess)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

		//UE_LOG(LogOnlineGame, Verbose, TEXT("Num Search Results: %d"), SearchSettings->SearchResults.Num());
		for (int32 i = 0; i < SearchSettings->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& SearchResult = SearchSettings->SearchResults[i];
			DumpSession(&SearchResult.Session);
		}

		FindSessionsCompleteEvent.Broadcast(isSuccess);
	}
}

void AGameSessionSV::OnStartOnlineGameComplete(FName sessionName, bool isSuccess)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid()) {
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
	}
}

EOnlineAsyncTaskState::Type AGameSessionSV::GetFindResultStatus(int32 &idx, int32 &count)
{
	idx = 0;
	count = 0;

	if (SearchSettings.IsValid())
	{
		if (SearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			idx = CurrentSessionParams.BestSessionIdx;
			count = SearchSettings->SearchResults.Num();
		}
		return SearchSettings->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

bool AGameSessionSV::JoinSession(TSharedPtr<const FUniqueNetId> userId, FName sessionname, const FOnlineSessionSearchResult &desiredSession)
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		OnJoinSessionCompleteDelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		OnStartSessionCompleteDelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
		return sessionInterface->JoinSession(0, sessionname, desiredSession);
	}		

	return false;
}

void AGameSessionSV::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	//CurrSessionName = sessionName;

	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid()) {
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	JoinSessionCompleteEvent.Broadcast(sessionName, result);
}


void AGameSessionSV::OnRegisterPlayerComplete(FName sessionName, const TArray< TSharedRef<const FUniqueNetId> >& playersNetId, bool wasSuccess)
{
	UGameInstanceSV *ins = (UGameInstanceSV*)GetGameInstance();
	if (ins)
	{
		ASVBattleMainMenu *mm = ins->MainMenu.Get();
		if (mm) {
			mm->RefreshPlayerList();
		}
	}
}

void AGameSessionSV::HandleMatchHasStarted()
{
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid() && HostSettings.IsValid()) 
	{
		UE_LOG(LogTemp, Log, TEXT("Starting session %s on server"), *GameSessionName.ToString());
		OnStartSessionCompleteDelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
		sessionInterface->StartSession(GameSessionName);
	}

	//FOnlineSessionStartComplete CompletionDelegate	= FOnlineSessionStartComplete::CreateUObject(this, &AGameSession::OnStartSessionComplete);
	//UOnlineEngineInterface::Get()->StartSession(GetWorld(), GameSessionName, OnStartSessionCompleteDelegate)
}

void AGameSessionSV::HandleMatchHasEnded()
{
	// start online game locally and wait for completion
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid())
	{
		// tell the clients to end
		for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
		{
			APlayerController* pc = Cast<APlayerController>(*it);
			if (pc && !pc->IsLocalPlayerController())
			{
				//pc->ClientEndOnlineGame();
			}
		}

		// server is handled here
		UE_LOG(LogOnlineGame, Log, TEXT("Ending session %s on server"), *GameSessionName.ToString());
		sessionInterface->EndSession(GameSessionName);
	}
}