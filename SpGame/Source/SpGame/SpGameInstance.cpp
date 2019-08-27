// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameInstance.h"
#include "SpGame.h"
#include "SpGameModeBattle.h"
#include "Character/SpCharacter.h"
#include "Character/AbilityObj.h"
#include "Res/ResLibrary.h"
#include "SpInputPreprocessor.h"
#include "GUI/DlgBase.h"
#include "GUI/DlgBattleResult.h"

#include <PlatformFileManager.h>
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

#include <splibclient/luainterface.h>
#include <splibclient/lua/lua.h>
#include "client/dllclient.h"

extern void LoadMap(const char *path);
extern void DoRealCmd(const summer::BattleRealCmdData *cmd);

UGameInstance *GGameInstance = nullptr;

DECLARE_LOG_CATEGORY_EXTERN(LogLua, Log, All);
DEFINE_LOG_CATEGORY(LogLua);

#if 0
static bool LuaLoader(const char *module)
{
	FString path = FPaths::GameDir() + "Content/Script/" + FString(module) + ".lua";
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		FString buffer;
		FFileHelper::LoadFileToString(buffer, *path);

		spclient::LuaLoadString(TCHAR_TO_UTF8(*buffer));
		return true;
	}

	return false;
}
#endif

static void LuaLogger(const int i, const char *log)
{
	switch ((ELogVerbosity::Type)i)
	{
		default:;
		case ELogVerbosity::Log :
			UE_LOG(LogLua, Log, TEXT("%s"), *FString(log));
		break;
	}
}

static EntityInterface *EntityFactory(ENTITY_TYPE type, int job, unsigned entity_id, int faction)
{
	AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	if (GameMode) 
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		switch (type)
		{
		case ENTITY_CHAR:
		{
			const JobInfo &jobInfo = spclient::getJobInfo(job);
			const FLookRow &lookRow = UResLibrary::GetLookByID(jobInfo.nLook);
			if (lookRow.PawnClass == nullptr) {
				UE_LOG(LogTemp, Error, TEXT("Can't find PawnClass of Look = %d."), job);
			}
			else
			{
				ASpCharacter *Pawn = GSpWorld->SpawnActor<ASpCharacter>(lookRow.PawnClass, SpawnInfo);
				if (Pawn)
				{
					Pawn->EntityId = entity_id;
					Pawn->Faction = faction;
					Pawn->SetLook(jobInfo.nLook);
					Pawn->JobId = job;
					//Pawn->Init(FVector(), FRotator());
					return (EntityInterface *)Pawn->GetEntityComponent();
				}
			}			
		}
			break;
		case ENTITY_SKILLOBJ:
		{
			AAbilityObj *Obj = GSpWorld->SpawnActor<AAbilityObj>(AAbilityObj::StaticClass(), SpawnInfo);
			if (Obj) 
			{
				Obj->EntityId = entity_id;
				Obj->Faction = faction;
				return (EntityInterface *)Obj->GetEntityComponent();
			}
		}
			break;
		default:
			break;
		}
	}
	return NULL;
}

const LevelConfig *LoadLevelRes (int nLevelId)
{
	AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	if (GameMode) {
		return &GameMode->LevelCfg;
	}
	
	static LevelConfig cfg;
	if (cfg.acts.empty())
	{
		cfg.acts.resize(8);
		for (int j = 0; j < cfg.acts.size(); ++j)
		{
			LevelAction &act = cfg.acts[j];

			act.nFrame = j;
			act.nType = LA_CREATE_CHAR;

			act.fPosX = 0;
			act.fPosY = 0;
			act.fPosZ = 100;

			act.fPitch = 0;
			act.fYaw = 0;
			act.fRoll = 0;
			act.nPawnId = j;
		}
	}	

	return &cfg;
}

void GetCurrMapInfo(char *path, bool &battle_mode)
{
	path[0] = 0;

	UWorld *world = GGameInstance ? GGameInstance->GetWorld() : nullptr;
	if (world)
	{
		FString file = world->GetOutermost()->FileName.ToString();
		int pie = file.Find(TEXT("UEDPIE_"), ESearchCase::CaseSensitive);
		if (pie >= 0) 
		{
			int name = file.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, pie);
			if (name >= 0)
			{
				FString file_path = file.Mid(0, pie);
				FString file_name = file.Mid(name + 1, file.Len() - (name + 1));
				strcpy(path, TCHAR_TO_UTF8(*file_path));
				strcat(path, TCHAR_TO_UTF8(*file_name));
			}
		}
		else {
			strcpy(path, TCHAR_TO_UTF8(*file));
		}		

		AGameModeBattle *gm_battle = Cast<AGameModeBattle>(world->GetAuthGameMode());
		if (gm_battle) {
			battle_mode = true;
		}
	}
}

void ComposeRotators(float aX, float aY, float aZ, float bX, float bY, float bZ, float *Pitch, float *Yaw, float *Roll)
{
	FRotator rotator = UKismetMathLibrary::ComposeRotators(FVector(aX, aY, aZ).Rotation(), FVector(bX, bY, bZ).Rotation());
	if (Pitch)
		*Pitch = rotator.Pitch;

	if (Yaw)
		*Yaw = rotator.Yaw;

	if (Roll)
		*Roll = rotator.Roll;
}

void Vector2Rotator(float x, float y, float z, float *Pitch, float *Yaw, float *Roll)
{
	FRotator rotator = FVector(x, y, z).Rotation();
	if (Pitch)
		*Pitch = rotator.Pitch;

	if (Yaw)
		*Yaw = rotator.Yaw;

	if (Roll)
		*Roll = rotator.Roll;
}

void Rotator2Vector(float Pitch, float Yaw, float Roll, float *x, float *y, float *z)
{
	FVector loc = FRotator(Pitch, Yaw, Roll).Vector();
	if (x)
		*x = loc.X;

	if (y)
		*y = loc.Y;

	if (z)
		*z = loc.Z;
}

int SprayMaskCount()
{
	return FSprayMaskRes::Get().GetMaskCount();
}
int SprayColorCount()
{
	return FSprayColorRes::Get().GetColorCount();
}

void GetBattleResult(float *atkerScore, float *deferScore)
{
	AGameModeBattle *GameMode = nullptr;
	if (GSpWorld) {
		GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	}

	if (atkerScore) {
		*atkerScore = GameMode ? GameMode->AtkerScore : 0;
	}

	if (deferScore) {
		*deferScore = GameMode ? GameMode->DeferScore : 0;
	}
}

void GetViewportForwardDir(float *dir)
{
	if (GSpWorld)
	{
		APlayerController *pc = GSpWorld->GetGameInstance()->GetFirstLocalPlayerController(nullptr);
		if (pc) 
		{
			FVector v = pc->GetActorForwardVector();
			dir[0] = v.X;
			dir[1] = v.Y;
			dir[2] = v.Z;
		}
	}
}

static void OnServerError(int code)
{
	if (code < summer::RETURN_MAX) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString(UTF8_TO_TCHAR(summer::MSGRETURNTYPE_STR[code])));
	}
}

static void OnNetEvent(const Uint8 *lpPack, int nLength)
{
	Uint16 type = *(Uint16*)lpPack;
	switch (type)
	{
	case summer::GMM2C_DUEL_REAL_MATCH:
		break;
	case summer::GMM2C_PLAYER_BATTLE_RESULT:
		if (GSpWorld)
		{
			AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
			if (GameMode)
			{
				if (!UDlgBattleResult::IsInstanced()) 
				{
					UDlgBattleResult::GetInstance()->Show();
					UDlgBattleResult::GetInstance()->UpdateScore(GameMode->AtkerScore, GameMode->DeferScore);
					SwitchInputMode(true, true);
				}
			}
		}
		break;
	default:
		break;
	}
}

static void draw_debug_line(const float *from, const float *to, const int *clr, float life)
{
	if (GSpWorld->GetWorld()) {
		DrawDebugLine(GSpWorld->GetWorld(), FVector(from[0], from[1], from[2]), FVector(to[0], to[1], to[2]), FColor(clr[0], clr[1], clr[2], clr[3]), life < 0.001f, life);
	}	
}

static void draw_debug_point(const float *from, float size, const int *clr, float life)
{
	if (GSpWorld->GetWorld()) {
		DrawDebugPoint(GSpWorld->GetWorld(), FVector(from[0], from[1], from[2]), size, FColor(clr[0], clr[1], clr[2], clr[3]), life < 0.001f, life);
	}
}

static char *load_file_content(const char *file, unsigned *len)
{
	char *ret = nullptr;
	UBinAsset *ba = LoadObject<UBinAsset>(GSpWorld->GetWorld(), UTF8_TO_TCHAR(file));
	if (ba)
	{
		ret = (char*)malloc(ba->Content.Num());
		if (ret) {
			memcpy(ret, &ba->Content[0], ba->Content.Num());
		}
		
		if (len) {
			*len = ba->Content.Num();
		}
	}

	return ret;
}

extern void spray_callback(const float *loc, const float *normal, const float *dir, int faction, const char *mask, const float *size);
extern int get_spray_faction_callback(const float *pt, int face, void *geomUserdata);

USpGameInstance::USpGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		GGameInstance = this;
	}
}

void USpGameInstance::Init()
{
	Super::Init();

	InputProcessor = MakeShareable(new FSpInputPreprocessor());
	FSlateApplication::Get().SetInputPreProcessor(true, InputProcessor);

	spclient::ClientCB clientCB;
	memset(&clientCB, 0, sizeof(clientCB));
	clientCB.onServerError = OnServerError;
	clientCB.onNetEvent = OnNetEvent;
	clientCB.loadFile = &load_file_content;
	clientCB.getViewportForward = &GetViewportForwardDir;
	clientCB.spray = &spray_callback;
	clientCB.getSprayFaction = &get_spray_faction_callback;
	clientCB.loadMap = LoadMap;
	clientCB.getCurrMapInfo = &GetCurrMapInfo;
	clientCB.doRealCmd = DoRealCmd;
	clientCB.luaLogger = LuaLogger;
	clientCB.entityFactory = EntityFactory;
	clientCB.loadLevelRes = LoadLevelRes;
	clientCB.drawLine = &draw_debug_line;
	clientCB.drawPoint = &draw_debug_point;
	clientCB.sprayMaskCount = SprayMaskCount;
	clientCB.sprayColorCount = SprayColorCount;
	clientCB.getBattleResult = GetBattleResult;
	clientCB.composeRotators = ComposeRotators;
	clientCB.vector2Rotator = Vector2Rotator;
	clientCB.rotator2Vector = Rotator2Vector;

	spclient::RegisterClientCB(clientCB);

	FString contentPath = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());
	FString lang = TEXT("zh-CN");
	spclient::init(TCHAR_TO_UTF8(*contentPath), TCHAR_TO_UTF8(*lang));
}

void USpGameInstance::Shutdown()
{
	InputProcessor = nullptr;
	spclient::shutdown();
	Super::Shutdown();
}

