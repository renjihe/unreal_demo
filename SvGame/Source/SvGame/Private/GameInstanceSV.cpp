// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameInstanceSV.h"
#include "OmniControllerPluginFunctionLibrary.h"
#include "HVInterface.h"
#include "OnlineSessionClientSV.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include <OnlinePresenceInterface.h>
#include <OnlineSubsystem.h>
#include <OnlineSubsystemUtils.h>
#include "GameSessionSV.h"

GameSVState::State UGameInstanceSV::CurrState = GameSVState::None;

UGameInstanceSV::UGameInstanceSV()
	: Super()
{	
	CurrState = GameSVState::None;

	MaxDecal = 30;
}

void UGameInstanceSV::Init()
{
	Super::Init();

	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);

	UHvInterface::init(1019, "01ec17dac7140c0fbe936ee128310000", "omni=1");

	DecalArr.Reset();
}

void UGameInstanceSV::Shutdown()
{
	DestroyCurrSession();

	Super::Shutdown();
}

TSubclassOf<UOnlineSession> UGameInstanceSV::GetOnlineSessionClass()
{
	return UOnlineSessionClientSV::StaticClass();
}

AGameSession* UGameInstanceSV::GetGameSession()
{
	const UWorld *world = GetWorld();
	if (world)
	{
		AGameModeBase *gameMode = world->GetAuthGameMode();
		if (gameMode) {
			return gameMode->GameSession;
		}
	}

	return NULL;
}

bool UGameInstanceSV::HostGame(ULocalPlayer* localPlayer, const FString& gameType, const FString& inTravelURL)
{
	// host LAN-game map;
	FString t = TEXT("/Game/Levels/level_02/Sv_Level_02?game=FFA?listen?bIsLanMatch");
	GetWorld()->ServerTravel(t);

	AGameSessionSV *session = Cast<AGameSessionSV>(GetGameSession());
	if (session)
	{
		// add callback delegate for completion
		OnCreatePresenceSessionCompleteDelegateHandle = session->CreatePresenceSessionCompleteEvent.AddUObject(this, &UGameInstanceSV::OnCreatePresenceSessionComplete);

		TravelURL = inTravelURL;

		bool const isLanMatch = inTravelURL.Contains(TEXT("?bIsLanMatch"));

		//determine the map name from the travelURL
		const FString& mapNameSubStr = "/Game/Maps/";
		const FString& choppedMapName = TravelURL.RightChop(mapNameSubStr.Len());
		const FString& mapName = choppedMapName.LeftChop(choppedMapName.Len() - choppedMapName.Find("?game"));

		if (session->HostSession(localPlayer->GetPreferredUniqueNetId(), GameSessionName, gameType, mapName, isLanMatch, true, 4))
		{
			SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));
			return true;			
		}
	}

	return false;
}

bool UGameInstanceSV::FindGames(ULocalPlayer* playerOwner, bool lanMatch)
{
	AGameSessionSV *session = Cast<AGameSessionSV>(GetGameSession());
	if (session && playerOwner) 
	{
		session->FindSessionsCompleteEvent.RemoveAll(this);

		OnFindSessionsCompleteDelegateHandle = session->FindSessionsCompleteEvent.AddUObject(this, &UGameInstanceSV::OnFindSessionsComplete);

		session->FindSessions(playerOwner->GetPreferredUniqueNetId(), GameSessionName, lanMatch, true);
		return true;
	}

	return false;
}

void UGameInstanceSV::OnCreateSessionComplete()
{
}

void UGameInstanceSV::OnCreatePresenceSessionComplete(FName sessionName, bool wasSuccessful)
{
	AGameSessionSV *session = Cast<AGameSessionSV>(GetGameSession());
	if (session) 
	{
		session->CreatePresenceSessionCompleteEvent.Remove(OnCreatePresenceSessionCompleteDelegateHandle);

		FinishSessionCreation(wasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);

		//if (!OnJoinsessionCompleteDelegateHandle.IsValid()) {
		//	OnJoinsessionCompleteDelegateHandle = session->JoinSessionCompleteEvent.AddUObject(this, &UGameInstance_SV::OnJoinSessionComplete);
		//}		
	}
}

void UGameInstanceSV::FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		//SendPlayTogetherInvites();
		ASVBattleMainMenu *menu = MainMenu.Get();
		if (menu) {
			menu->SetState(GameSVState::CreatedHost);
		}
	}
	else
	{
		UHvInterface::MsgBox(FText::FromString(TEXT("Error")), FText::FromString(TEXT("FinishSessionCreation result failed.")), nullptr);
	}
}


void UGameInstanceSV::OnFindSessionsComplete(bool wasSuccessful)
{
	AGameSessionSV *session = Cast<AGameSessionSV>(GetGameSession());
	if (session)
	{
		session->FindSessionsCompleteEvent.Remove(OnFindSessionsCompleteDelegateHandle);
	}
}


void UGameInstanceSV::SendPlayTogetherInvites()
{
	/*
	const IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	if (PlayTogetherInfo.UserIndex != -1)
	{
		for (const ULocalPlayer* LocalPlayer : LocalPlayers)
		{
			if (LocalPlayer->GetControllerId() == PlayTogetherInfo.UserIndex)
			{
				TSharedPtr<const FUniqueNetId> PlayerId = LocalPlayer->GetPreferredUniqueNetId();
				if (PlayerId.IsValid())
				{
					// Automatically send invites to friends in the player's PS4 party to conform with Play Together requirements
					for (const TSharedPtr<const FUniqueNetId>& FriendId : PlayTogetherInfo.UserIdList)
					{
						SessionInterface->SendSessionInviteToFriend(*PlayerId.ToSharedRef(), GameSessionName, *FriendId.ToSharedRef());
					}
				}

			}
		}

		PlayTogetherInfo = FShooterPlayTogetherInfo();
	}
	*/
}

void UGameInstanceSV::SetPresenceForLocalPlayers(const FVariantData& PresenceData)
{
	const auto presence = Online::GetPresenceInterface();
	if (presence.IsValid())
	{
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			const TSharedPtr<const FUniqueNetId> userId = LocalPlayers[i]->GetPreferredUniqueNetId();

			if (userId.IsValid())
			{
				FOnlineUserPresenceStatus presenceStatus;
				presenceStatus.Properties.Add(DefaultPresenceKey, PresenceData);

				presence->SetPresence(*userId, presenceStatus);
			}
		}
	}
}

void UGameInstanceSV::OnRegisterLocalPlayerComplete(const FUniqueNetId& playerId, EOnJoinSessionCompleteResult::Type result)
{
	FinishSessionCreation(result);
}

bool UGameInstanceSV::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	auto session = Cast<AGameSessionSV>(GetGameSession());
	if (session) 
	{
		//if (!OnJoinsessionCompleteDelegateHandle.IsValid())
		{
			OnJoinsessionCompleteDelegateHandle = session->JoinSessionCompleteEvent.AddUObject(this, &UGameInstanceSV::OnJoinSessionComplete);
		}

		return session->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), session->SessionName, SearchResult);
	}

	return false;
}

void UGameInstanceSV::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	auto session = Cast<AGameSessionSV>(GetGameSession());
	if (session) {
		session->JoinSessionCompleteEvent.Remove(OnJoinsessionCompleteDelegateHandle);
	}

	if (result == EOnJoinSessionCompleteResult::Success)
	{
		auto sessionInterface = Online::GetSessionInterface(GetWorld());

		FString url;
		if (sessionInterface.IsValid() && sessionInterface->GetResolvedConnectString(GameSessionName, url))
		{
			UE_LOG(LogTemp, Display, TEXT("JoinSessionComplete, traval to %s"), *url);
			GetFirstLocalPlayerController(GetWorld())->ClientTravel(url, TRAVEL_Absolute);

			ASVBattleMainMenu *mm = MainMenu.Get();
			if (mm) {
				mm->SetState(GameSVState::Joined);
			}
		}

		//sessionInterface->RegisterLocalPlayer(*LocalPlayers[0]->GetPreferredUniqueNetId(), sessionName
		//					, FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UGameInstance_SV::OnRegisterJoiningLocalPlayerComplete));
	}
	else
	{
		ASVBattleMainMenu *mm = MainMenu.Get();
		if (mm) {
			mm->SetState(GameSVState::JoinFailed);
		}
	}
}

void UGameInstanceSV::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& playerId, EOnJoinSessionCompleteResult::Type result)
{
	//FinishSessionJoin(result);
 //	if (result == EOnJoinSessionCompleteResult::Success)
	//{
	//	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	//	FString url;
	//	if (sessionInterface.IsValid() && sessionInterface->GetResolvedConnectString(GameSessionName, url))
	//	{
	//		UE_LOG(LogTemp, Display, TEXT("JoinSessionComplete, traval to %s"), *url);
	//		GetFirstLocalPlayerController(GetWorld())->ClientTravel(url, TRAVEL_Absolute);

	//		ASVBattleMainMenu *mm = MainMenu.Get();
	//		if (mm) {
	//			mm->SetState(GameSVState::Joined);
	//		}
	//	}		
	//}
}

void UGameInstanceSV::DestroyCurrSession()
{
	AGameSession *session = GetGameSession();
	auto sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface.IsValid() && session) {
		sessionInterface->DestroySession(session->SessionName);
	}
}

void UGameInstanceSV::AddDecalItem(FDecalItem decal)
{
	DecalArr.Add(decal);

	if (DecalArr.Num() >= MaxDecal)
	{
		float time = GetWorld()->GetTimeSeconds();
		//if (DecalArr[0].AutoDestroyTime > time) 
		{
			UDecalComponent *compo = DecalArr[0].Comp.Get();
			if (IsValid(compo)) {
				compo->DestroyComponent();
			}			
		}
		DecalArr.RemoveAt(0);
	}
}