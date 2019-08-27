// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "SVBattleMainMenu.h"
#include <OnlineSessionInterface.h>
#include <OnlineKeyValuePair.h>
#include "GameInstanceSV.generated.h"

#define USE_HVSDK_CPP 1

class AGameSessionSV;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAlreadyinitiatedOmni);

namespace GameSVState
{
	enum State {
		None,
		FindingGames,
		Found,
		JoinFailed,
		Joining,
		Joined,
		CreatingHost,
		CreatedHost,
		StartingHost,		
	};
};

USTRUCT()
struct FDecalItem
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<UDecalComponent> Comp;
	//UDecalComponent* Comp;
	float AutoDestroyTime;
};

/**
 * 
 */
UCLASS()
class SVGAME_API UGameInstanceSV : public UGameInstance
{
	GENERATED_BODY()

	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;
	FDelegateHandle OnJoinsessionCompleteDelegateHandle;
public:
	UGameInstanceSV();

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;

	static GameSVState::State CurrState;
	FString TravelURL;
	TWeakObjectPtr<ASVBattleMainMenu> MainMenu;

	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type result);

	void SetPresenceForLocalPlayers(const FVariantData& presenceData);

	void SendPlayTogetherInvites();
public:
	AGameSession* GetGameSession();

	void OnCreateSessionComplete();
	void OnCreatePresenceSessionComplete(FName sessionName, bool wasSuccessful);
	void OnFindSessionsComplete(bool wasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);
	
	void OnRegisterLocalPlayerComplete(const FUniqueNetId& playerId, EOnJoinSessionCompleteResult::Type result);
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& playerId, EOnJoinSessionCompleteResult::Type result);

	bool HostGame(ULocalPlayer* localPlayer, const FString& gameType, const FString& inTravelURL);
	bool FindGames(ULocalPlayer* playerOwner, bool lanMatch);

	virtual bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult) override;
	void DestroyCurrSession();

	UPROPERTY(EditDefaultsOnly)
	int MaxDecal;

	TArray<FDecalItem> DecalArr;

	void AddDecalItem(FDecalItem decal);
};
