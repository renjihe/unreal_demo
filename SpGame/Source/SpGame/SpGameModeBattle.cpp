// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameModeBattle.h"
#include "SpGame.h"
#include "Character/SpCharacter.h"
#include "SpPlayerStart.h"
#include "GUI/DlgBattleInfo.h"
#include "GUI/DlgBattleResult.h"
#include "GUI/SpHUD.h"
#include "Res/ResLibrary.h"
#include "SpPlayerController.h"

#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "client/dllclient.h"
#include "common/battledef.h"

using namespace summer;

extern UGameInstance *GGameInstance;

int32 GRandSeed = 0;
FAutoConsoleVariableRef GRandSeedCVar(
	TEXT("sp.BattleRandSeed"),
	GRandSeed,
	TEXT("Battle random seed.\n")
	TEXT(" default: 0"));

void LoadMap(const char *path)
{
	UWorld *world = GGameInstance->GetWorld();
	if (world) {
#if 1
		world->ServerTravel(FString(path));
#else
		APlayerController *controller = world->GetFirstPlayerController();
		controller->ClientTravel(FString(path), ETravelType::TRAVEL_Relative, true);
#endif
	}	
}

void DoRealCmd(const BattleRealCmdData *cmd)
{
	AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	if (GameMode) {
		GameMode->DoRealCmd(cmd);
	}
}

void GetSprayColor(FVector &AtkerColor, FVector &DeferColor)
{
	AGameModeBattle *GameMode = nullptr;
	if (GSpWorld) {
		GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	}

	if (GameMode)
	{
		AtkerColor = GameMode->AtkerColor;
		DeferColor = GameMode->DeferColor;
	}
	else {
		AtkerColor.Set(1, 0, 0);
		DeferColor.Set(0, 1, 0);
	}
}

FVector GetSprayColor(int faction)
{
	if (GSpWorld)
	{
		AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
		if (GameMode) {
			return faction ? GameMode->DeferColor : GameMode->AtkerColor;
		}
	}	
	return FVector(1, 0, 0);
}

FName GetAvatarName(AvatarPosition pos)
{
	switch (pos)
	{
	case Actor:
		return FName(*FString("Actor"));
		break;
	case Head:
		return FName(*FString("Head"));
		break;
	case Body:
		return FName(*FString("Body"));
		break;
	case Back:
		return FName(*FString("Back"));
		break;
	case LeftHand:
		return FName(*FString("LeftHand"));
		break;
	case RightHand:
		return FName(*FString("RightHand"));
		break;
	case Arm:
		return FName(*FString("Arm"));
		break;
	case Leg:
		return FName(*FString("Leg"));
		break;
	default:
		break;
	}
	return FName();
}

AGameModeBattle::AGameModeBattle(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	HUDClass = ASpHUD::StaticClass();
	PlayerControllerClass = ASpPlayerController::StaticClass();
}

//#define BATTLE_FPS 30
//#define BATTLE_FRAME_TICK	(1000/BATTLE_FPS)
//#define BATTLE_FRAME_SEC	(0.001f*BATTLE_FRAME_TICK)
void AGameModeBattle::BeginPlay()
{
	Super::BeginPlay();

	const FSprayColorRow &row = UResLibrary::GetSprayColorByID(spclient::getSprayColorGroupIndex());
	AtkerColor = FVector(row.AtkerColor.R / 255.0f, row.AtkerColor.G / 255.0f, row.AtkerColor.B / 255.0f);
	DeferColor = FVector(row.DeferColor.R / 255.0f, row.DeferColor.G / 255.0f, row.DeferColor.B / 255.0f);
	AtkerScore = 0;
	DeferScore = 0;

	int count = 0;
	for (TActorIterator<ASpPlayerStart> It(GetWorld()); It; ++It) {
		count++;
	}
	LevelCfg.acts.resize(count);

	int j = 0;
	for (TActorIterator<ASpPlayerStart> It(GetWorld()); It; ++It, ++j) 
	{
		LevelAction &act = LevelCfg.acts[j];

		act.nFrame = It->Frame;
		act.nType = (LevelActionType)It->Type;
		FVector loc = It->GetActorLocation();
		FRotator rot = It->GetActorRotation();

		act.fPosX = loc.X;
		act.fPosY = loc.Y;
		act.fPosZ = loc.Z;

		act.fPitch = rot.Pitch;
		act.fYaw = rot.Yaw;
		act.fRoll = rot.Roll;
		act.nPawnId = It->PawnId;

		act.nRebirthTime = It->RebirthTime;
	}

	spclient::onBattleLoadDone();
}

void AGameModeBattle::OnInited()
{
	Super::OnInited();

	UDlgBattleInfo::GetInstance()->Show();
	SwitchInputMode(false);
}

void AGameModeBattle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	spclient::onBattleCancel();

	Super::EndPlay(EndPlayReason);
}

void AGameModeBattle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UDlgBattleInfo::GetInstance()->UpdateFrame(DeltaSeconds);
	if (UDlgBattleResult::IsInstanced()) {
		UDlgBattleResult::GetInstance()->UpdateFrame(DeltaSeconds);
	}

	UDlgBattleInfo::GetInstance()->UpdateMPRange(spclient::getBattleUserMpExRate());
}

void AGameModeBattle::StartPlay()
{
	Super::StartPlay();
}

void AGameModeBattle::Init(bool bStartUp)
{
	if (bInit)
		return;

	bInit = true;
}

AGameModeBattle::TConstIterator AGameModeBattle::CreatePawnConstIterator()
{
	return Pawns.CreateConstIterator();
}

void AGameModeBattle::DoRealCmd(const BattleRealCmdData *cmd)
{
	/*ASpCharacter *Pawn = ChoosePawnById(cmd->nPlayer + 1);
	if (nullptr == Pawn)
		return;

	switch (cmd->nCmd)
	{
	case REALCMD_KEY_PRESS:
		if (cmd->nX == BATTLE_CMD_KEY_W)
			Pawn->MoveForwardImpl(1.0, cmd->fY);
		else if (cmd->nX == BATTLE_CMD_KEY_S)
			Pawn->MoveForwardImpl(-1.0, cmd->fY);
		else if (cmd->nX == BATTLE_CMD_KEY_A)
			Pawn->MoveRightImpl(-1.0, cmd->fY);
		else if (cmd->nX == BATTLE_CMD_KEY_D)
			Pawn->MoveRightImpl(1.0, cmd->fY);
		break;
	case REALCMD_KEY_RELEASE:
		break;
	case REALCMD_MOUSE_PRESS:
		if (cmd->nX == BATTLE_CMD_MOUSE_LEFT)
			Pawn->MouseLeftPressImpl();
		else if (cmd->nX == BATTLE_CMD_MOUSE_RIGHT)
			Pawn->MouseRightPressImpl();
		break;
	case REALCMD_MOUSE_RELEASE:
		if (cmd->nX == BATTLE_CMD_MOUSE_LEFT)
			Pawn->MouseLeftReleaseImpl();
		else if (cmd->nX == BATTLE_CMD_MOUSE_RIGHT)
			Pawn->MouseRightReleaseImpl();
		break;
	case REALCMD_GIVEUP:
		break;
	case REALCMD_KILL:
		break;
	case REALCMD_CHECK:
		break;
	case REALCMD_BATTLE_RESULT:
		break;
	case REALCMD_PING:
		break;
	default:
		break;
	}*/
}
