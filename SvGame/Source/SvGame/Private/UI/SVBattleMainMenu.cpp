#include "SvGame.h"
#include "HVStringRes.h"
#include "SVBattleMainMenu.h"
#include "GameInstanceSV.h"
#include "GameSessionSV.h"
#include "ListBox.h"
#include "EngineGlobals.h"
#include "UMG.h"
#include "Components/WidgetComponent.h"
//#include <SBoxPanel.h>
//#include <VerticalBox.h>
#include <UserWidget.h>
#include <TextBlock.h>
//#include <ListView.h>

ASVBattleMainMenu::ASVBattleMainMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAlwaysOnCamera = false;
	WinPlayerList = WinGameList = NULL;

	UWorld *world = GetWorld();
	static ConstructorHelpers::FObjectFinder<UClass> win_bp(TEXT("Class'/Game/Widgets/BattleMainMenu.BattleMainMenu_C'"));
	if (win_bp.Object && !HasAnyFlags(RF_ClassDefaultObject) && world) {
		RootWidget = (UUserWidget*)CreateWidget<UUserWidget>(world, win_bp.Object);
		WidgetComponent->SetWidget(RootWidget);
	}
}

ASVBattleMainMenu::~ASVBattleMainMenu()
{
}


void ASVBattleMainMenu::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if (!HasAnyFlags(RF_ClassDefaultObject) && GetWorld() != nullptr)
	{
		UGameInstanceSV *ins = Cast<UGameInstanceSV>(GetGameInstance());
		if (ins) {
			ins->MainMenu = this;
		}
	}

}

void ASVBattleMainMenu::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitWidget();

	if (UGameInstanceSV::CurrState == GameSVState::None) {
		SetState(GameSVState::FindingGames);
	}
	else {
		Refresh();
	}	
}


void ASVBattleMainMenu::BeginPlay()
{
	Super::BeginPlay();

	//InitWidget();
}

#define INIT_MENU_BUTTON(name, cb) LookupButton(TEXT(name), TEXT(name), &ASVBattleMainMenu::##cb, TEXT(#cb))

UButton* ASVBattleMainMenu::LookupButton(const TCHAR *name, const TCHAR *caption_strid, ASVBattleMainMenu::ClickCB cb, const TCHAR *cb_name)
{
	UButton *ret = Cast<UButton>(RootWidget->GetWidgetFromName(name));
	if (ret)
	{
		ret->OnClicked.__Internal_AddDynamic(this, cb, cb_name);

		UTextBlock *cap = Cast<UTextBlock>(ret->GetContentSlot()->Content);
		if (cap) {
			cap->SetText(HVSTRING(caption_strid));
		}
	}

	return ret;
}

void ASVBattleMainMenu::InitWidget()
{
	WidgetComponent->InitWidget();

	RootWidget = WidgetComponent->GetUserWidgetObject();
	if (RootWidget)
	{
		UButton *btn1 = INIT_MENU_BUTTON("HostBtn", OnClickHost);
		UButton *btn2 = INIT_MENU_BUTTON("RefreshBtn", OnClickRefresh);
		UButton *btn3 = INIT_MENU_BUTTON("StartBtn", OnClickStart);
		UButton *btn4 = INIT_MENU_BUTTON("SearchBtn", OnClickSearch);
		UButton *btn5 = INIT_MENU_BUTTON("JoinBtn", OnClickJoin);

		WinPlayerList = RootWidget->GetWidgetFromName("PlayerListWin");
		WinGameList = RootWidget->GetWidgetFromName("GameListWin");

		PlayerList = Cast<UListBox>(RootWidget->GetWidgetFromName("PlayerListBox"));
		if (PlayerList)
		{
			PlayerList->OnGenerateRowEvent.BindDynamic(this, &ASVBattleMainMenu::MakeRowPlayerList);
			PlayerList->OnSelectionChangedEvent.BindDynamic(this, &ASVBattleMainMenu::OnSelectionChangedPlayerList);
		}

		GameList = Cast<UListBox>(RootWidget->GetWidgetFromName("GameListBox"));
		if (GameList)
		{
			GameList->OnGenerateRowEvent.BindDynamic(this, &ASVBattleMainMenu::MakeRowGameList);
			GameList->OnSelectionChangedEvent.BindDynamic(this, &ASVBattleMainMenu::OnSelectionChangedGameList);
			GameList->OnDoubleClickEvent.BindDynamic(this, &ASVBattleMainMenu::OnDoubleClickGameList);
		}
		

		//UPanelWidget *win = Cast<UPanelWidget>(RootWidget->GetRootWidget());
		//if (win) 
		//{
		//	StateWidget = NewObject<UTextBlock>(GetWorld(), UTextBlock::StaticClass()); //CreateWidget<UTextBlock>(GetWorld(), UTextBlock::StaticClass());
		//	win->AddChild(StateWidget);
		//}
		StateWidget = Cast<UTextBlock>(RootWidget->GetWidgetFromName("Status"));
	}
}

void ASVBattleMainMenu::OnClickHost()
{
	SetState(GameSVState::CreatingHost);
}

void ASVBattleMainMenu::OnClickRefresh()
{
	SetState(GameSVState::FindingGames);
}

void ASVBattleMainMenu::OnClickStart()
{
	SetState(GameSVState::StartingHost);
	/*
	//GEngine->LoadMap("/Game/Levels/Sv_Mock_01");
	FURL url = TEXT("/Game/Levels/Sv_Mock_01");// (*FString::Printf(TEXT("%s"), *MapName));

	FString err;
	EBrowseReturnVal::Type ret = GEngine->Browse(*WorldContext, url, err);
	if (ret != EBrowseReturnVal::Success)
	{
		UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *url.ToString(), *err));
	}
	*/
}

void ASVBattleMainMenu::OnClickSearch()
{
	SetState(GameSVState::FindingGames);
}

void ASVBattleMainMenu::OnClickJoin()
{
	if (!CurrSelectServer.IsEmpty()) {
		SetState(GameSVState::Joining);
	}	
}

AGameSessionSV* ASVBattleMainMenu::GetGameSession()
{
	UGameInstanceSV *ins = Cast<UGameInstanceSV>(GetGameInstance());
	if (ins) {
		return Cast<AGameSessionSV>(ins->GetGameSession());
	}

	return nullptr;
}

void ASVBattleMainMenu::SetState(int state)
{
	if (state == (int)UGameInstanceSV::CurrState) {
		return;
	}

	UGameInstanceSV::CurrState = (GameSVState::State)state;

	UGameInstanceSV *ins = Cast<UGameInstanceSV>(GetGameInstance());
	AGameSessionSV *sess = GetGameSession();
	if (nullptr == sess) {
		return;
	}

	switch (state)
	{
	case GameSVState::FindingGames:
		ins->DestroyCurrSession();
		sess->FindSessions(ins->GetLocalPlayerByIndex(0)->GetPreferredUniqueNetId(), GameSessionName, true, true);
		break;
	case GameSVState::Joining:
		{
			const TArray<FOnlineSessionSearchResult> &res = sess->GetSearchResults();
			if (res.Num())
			{
				int32 i = FCString::Atoi(*CurrSelectServer);
				if (i >= res.Num()) {
					i = 0;
				}

				const FOnlineSessionSearchResult& ri = res[i];
				ins->JoinSession(ins->GetFirstGamePlayer(), ri);
			}
		}
		break;
	case GameSVState::Joined:
		break;
	case GameSVState::CreatingHost:
		{
			FString url = FString::Printf(TEXT("/Game/Levels/%s?game=%s%s%s?%s=%d%s"), TEXT("Sv_Mock_01"), TEXT("FFA"), TEXT("?listen"), TEXT("?bIsLanMatch"), TEXT("Bots"), 4, TEXT("?DemoRec"));
			ins->HostGame(ins->GetFirstGamePlayer(), TEXT("FFA"), url);
		}		
		break;
	case GameSVState::CreatedHost:
		break;
	case GameSVState::StartingHost:
		GetWorld()->ServerTravel(ins->TravelURL);
		break;
	}

	Refresh();
}

void ASVBattleMainMenu::Tick(float DeltaSeconds)
{
	if (UGameInstanceSV::CurrState == GameSVState::CreatedHost)
	{
		static float timer = 0;
		if ((timer += DeltaSeconds) > 0.5f) {
			RefreshPlayerList();
			timer = 0;
		}
		return;
	}

	if (UGameInstanceSV::CurrState != GameSVState::FindingGames) {
		return;
	}

	bool bFinishSearch = true;
	AGameSessionSV *sess = GetGameSession();
	if (sess)
	{
		int32 idx, count;
		EOnlineAsyncTaskState::Type findResult = sess->GetFindResultStatus(idx, count);

		//UE_LOG(LogOnlineGame, Log, TEXT("ShooterSession->GetSearchResultStatus: %s"), EOnlineAsyncTaskState::ToString(SearchState));

		if (findResult == EOnlineAsyncTaskState::Done)
		{
			ServerList.Empty();

			const TArray<FOnlineSessionSearchResult> &res = sess->GetSearchResults();						
			for (int32 i = 0; i < res.Num(); ++i)
			{
				const FOnlineSessionSearchResult& ri = res[i];
				TSharedPtr<FServerEntry> svr = MakeShareable<FServerEntry>(new FServerEntry);

				svr->ServerName = ri.Session.OwningUserName;
				svr->Ping = FString::FromInt(ri.PingInMs);
				svr->CurrentPlayers = FString::FromInt(ri.Session.SessionSettings.NumPublicConnections
					+ ri.Session.SessionSettings.NumPrivateConnections
					- ri.Session.NumOpenPublicConnections
					- ri.Session.NumOpenPrivateConnections);
				svr->MaxPlayers = FString::FromInt(ri.Session.SessionSettings.NumPublicConnections
					+ ri.Session.SessionSettings.NumPrivateConnections);
				svr->SearchResultsIndex = i;

				ri.Session.SessionSettings.Get(SETTING_GAMEMODE, svr->GameType);
				ri.Session.SessionSettings.Get(SETTING_MAPNAME, svr->MapName);

				ServerList.Add(svr);
			}

			SetState(GameSVState::Found);
		}
	}
}

UWidget* ASVBattleMainMenu::MakeRowGameList(FString item)
{
	UTextBlock *ret = NewObject<UTextBlock>(GetWorld(), UTextBlock::StaticClass());
	int index = FCString::Atoi(*item);
	if (index < ServerList.Num()) {
		ret->SetText(FText::FromString(ServerList[index]->ServerName));
	}
	else {
		ret->SetText(FText::FromString(TEXT("Unkown")));
	}
	
	return ret;
}

void ASVBattleMainMenu::OnSelectionChangedGameList(FString item, ESelectInfo::Type selType)
{
	CurrSelectServer = item;
}

void ASVBattleMainMenu::OnDoubleClickGameList(FString item)
{
	CurrSelectServer = item;
	SetState(GameSVState::Joining);
}

void ASVBattleMainMenu::RefreshPlayerList()
{
	if (PlayerList)
	{
		PlayerList->Items.Empty();

		for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
		{
			APlayerController* pc = (*it).Get();
			if (pc)
			{
				TSharedPtr<FString> row = MakeShareable(new FString());
				*row = pc->GetName();

				PlayerList->Items.Add(row);
			}
		}

		PlayerList->RebuildList();
	}	
}

UWidget* ASVBattleMainMenu::MakeRowPlayerList(FString item)
{
	UTextBlock *ret = NewObject<UTextBlock>(GetWorld(), UTextBlock::StaticClass());
	ret->SetText(FText::FromString(item));
	return ret;
}

void ASVBattleMainMenu::OnSelectionChangedPlayerList(FString item, ESelectInfo::Type selType)
{

}

void ASVBattleMainMenu::Refresh()
{
	bool showFind = UGameInstanceSV::CurrState <= GameSVState::Joining;
	if (WinGameList) {
		WinGameList->SetVisibility(showFind ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (WinPlayerList) {
		WinPlayerList->SetVisibility(showFind ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}


	const char *state_text = "StateNone";
	switch (UGameInstanceSV::CurrState)
	{
	case GameSVState::FindingGames:
		state_text = "StateFindingGames";
		break;
	case GameSVState::Found:
		state_text = "StateFound";
		RefreshGameList();
		break;
	case GameSVState::JoinFailed:
		state_text = "StateJoinFailed";
		break;
	case GameSVState::Joining:
		state_text = "StateJoining";
		break;
	case GameSVState::Joined:
		state_text = "StateJoined";
		RefreshPlayerList();
		break;
	case GameSVState::CreatingHost:
		state_text = "StateCreatingHost";
		break;
	case GameSVState::CreatedHost:
		state_text = "StateCreatedHost";
		RefreshPlayerList();
		break;
	case GameSVState::StartingHost:
		state_text = "StateStartingHost";
		break;
	}

	if (StateWidget) {
		StateWidget->SetText(HVSTRING(state_text));
	}
}

void ASVBattleMainMenu::RefreshGameList()
{
	if (GameList)
	{
		GameList->Items.Empty();

		for (int i = 0; i < ServerList.Num(); ++i)
		{
			TSharedPtr<FString> row = MakeShareable(new FString());
			*row = FString::FormatAsNumber(i);

			GameList->Items.Add(row);
		}		

		GameList->RebuildList();
	}	
}
