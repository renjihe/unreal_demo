#pragma once
#include "HVDlgBase.h"
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "Components/Widget.h"
#include <Components/ScrollBox.h>
#include "SVBattleMainMenu.generated.h"

class UUserWidget;
class UWidgetComponent;
class AActor;
class ASVBattleMainMenu;
class UButton;
class UVerticalBox;
class UTextBlock;
class UListBox;
class AGameSessionSV;

struct FServerEntry
{
	FString ServerName;
	FString CurrentPlayers;
	FString MaxPlayers;
	FString GameType;
	FString MapName;
	FString Ping;
	int32 SearchResultsIndex;
};

UCLASS()
class SVGAME_API ASVBattleMainMenu : public AHVDlgBase
{
	GENERATED_BODY()
public:
	ASVBattleMainMenu(const FObjectInitializer& ObjectInitializer);
	~ASVBattleMainMenu();

	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void SetState(int state);

	void RefreshPlayerList();
private:
	UWidget *WinPlayerList, *WinGameList;
	//UButton *_BtnHost, *_BtnSearch, *_BtnStart, *_BtnRefresh;	

	UTextBlock *StateWidget;

	void Refresh();

	UListBox *PlayerList, *GameList;
	void RefreshGameList();
	UFUNCTION()
	UWidget* MakeRowGameList(FString item);
	UFUNCTION()
	void OnSelectionChangedGameList(FString item, ESelectInfo::Type selType);
	UFUNCTION()
	void OnDoubleClickGameList(FString item);

	UFUNCTION()
	UWidget* MakeRowPlayerList(FString item);
	UFUNCTION()
	void OnSelectionChangedPlayerList(FString item, ESelectInfo::Type selType);

	void InitWidget();
	
	TArray<TSharedPtr<FServerEntry>> ServerList;
	FString CurrSelectServer;

	typedef void (ASVBattleMainMenu::*ClickCB)();
	UButton* LookupButton(const TCHAR *name, const TCHAR *caption_strid, ClickCB cb, const TCHAR *cb_name);

	UFUNCTION()
	void OnClickHost();
	UFUNCTION()
	void OnClickSearch();
	UFUNCTION()
	void OnClickStart();
	UFUNCTION()
	void OnClickRefresh();
	UFUNCTION()
	void OnClickJoin();

	AGameSessionSV* GetGameSession();
};
