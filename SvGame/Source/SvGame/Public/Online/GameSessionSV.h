// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameSession.h"
#include <OnlineSessionSettings.h>
#include <OnlineSessionInterface.h>
#include "GameSessionSV.generated.h"

struct FGameSessionParams
{
	/** Name of session settings are stored with */
	FName SessionName;
	/** LAN Match */
	bool IsLAN;
	/** Presence enabled session */
	bool IsPresence;
	/** Id of player initiating lobby */
	TSharedPtr<const FUniqueNetId> UserId;
	/** Current search result choice to join */
	int32 BestSessionIdx;

	FGameSessionParams()
		: SessionName(NAME_None)
		, IsLAN(false)
		, IsPresence(false)
		, BestSessionIdx(0)
	{
	}
};


/**
* General session settings
*/
class FOnlineSessionSettingsSV : public FOnlineSessionSettings
{
public:
	FOnlineSessionSettingsSV(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FOnlineSessionSettingsSV() {}
};

/**
* General search setting
*/
class FOnlineSessionSearchSV : public FOnlineSessionSearch
{
public:
	FOnlineSessionSearchSV(bool bSearchingLAN = false, bool bSearchingPresence = false);
	virtual ~FOnlineSessionSearchSV() {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
class FOnlineSessionSearchEmptyDedicatedSV : public FOnlineSessionSearchSV
{
public:
	FOnlineSessionSearchEmptyDedicatedSV(bool bSearchingLAN = false, bool bSearchingPresence = false);
	virtual ~FOnlineSessionSearchEmptyDedicatedSV() {}
};

//Params = SessionName, bWasSuccessful;
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStartSessionComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateSessionComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndSessionComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDestroySessionComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCancelMatchmakingComplete, FName, bool);
//DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindSessionsComplete, bool);
//DECLARE_MULTICAST_DELEGATE_OneParam(FOnCancelFindSessionsComplete, bool);


/**
 * AGameSession for SV;
 */
UCLASS()
class SVGAME_API AGameSessionSV : public AGameSession
{
	GENERATED_UCLASS_BODY()
public:
	//DECLARE_EVENT_TwoParams(AGameSessionSV, FOnCreatePresenceSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	//DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnJoinSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	//DECLARE_EVENT_OneParam(AGameSessionSV, FOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type /*Result*/);
	//DECLARE_EVENT(AGameSessionSV, FOnFindSessionsComplete);
	//DECLARE_EVENT_OneParam(AGameSessionSV, FOnFindSessionsComplete, bool /*bWasSuccessful*/);

private:
	FGameSessionParams SessionParams;
	FGameSessionParams CurrentSessionParams;
	TSharedPtr<class FOnlineSessionSettingsSV>	HostSettings;
	TSharedPtr<class FOnlineSessionSearchSV>	SearchSettings;	

	FOnCreateSessionComplete::FDelegate OnCreateSessionCompleteDelegate;
	FOnDestroySessionComplete::FDelegate OnDestroySessionCompleteDelegate;

	FOnFindSessionsComplete::FDelegate OnFindSessionsCompleteDelegate;
	FOnJoinSessionComplete::FDelegate OnJoinSessionCompleteDelegate;
	FOnRegisterPlayersComplete::FDelegate OnRegisterPlayerCompleteDelegate;

	FOnStartSessionComplete::FDelegate OnStartSessionCompleteDelegate;	

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnRegisterPlayerCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	void OnCreateSessionComplete(FName sessionName, bool isSuccess);
	void OnDestroySessionComplete(FName sessionName, bool isSuccess);
	void OnFindSessionsComplete(bool isSuccess);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);
	void OnRegisterPlayerComplete(FName sessionName, const TArray< TSharedRef<const FUniqueNetId> >& playersNetId, bool wasSuccess);
	void OnStartOnlineGameComplete(FName sessionName, bool isSuccess);
public:
	FOnCreateSessionComplete CreatePresenceSessionCompleteEvent;
	FOnJoinSessionComplete JoinSessionCompleteEvent;
	FOnFindSessionsComplete FindSessionsCompleteEvent;

public:
	//AGameSessionSV::AGameSessionSV(const FObjectInitializer& objectInitializer);

	bool HostSession(TSharedPtr<const FUniqueNetId> userId, FName inSessionName, const FString& gameType, const FString& mapName, bool isLan, bool isPresence, int32 maxNumPlayers);
	
	void FindSessions(TSharedPtr<const FUniqueNetId> userId, FName sessionName, bool isLan, bool isPresence);
	EOnlineAsyncTaskState::Type GetFindResultStatus(int32 &idx, int32 &count);
	inline const TArray<FOnlineSessionSearchResult>& GetSearchResults() const {
		return SearchSettings->SearchResults;
	}

	bool JoinSession(TSharedPtr<const FUniqueNetId> userId, FName sessionname, const FOnlineSessionSearchResult &desiredSession);

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
};
