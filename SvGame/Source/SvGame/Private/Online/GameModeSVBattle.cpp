// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameModeSVBattle.h"
#include "MajorCharacter.h"
#include "PlayerControllerSV.h"
#include "PlayerStateSV.h"
#include "PlayerStartSV.h"
#include "TargetPointSV.h"
#include "Character/MotionControllerProp.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AGameModeSVBattle::AGameModeSVBattle(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bUseSeamlessTravel = true;
	bInit = false;
	DefaultPawnClass = APawn::StaticClass();
}

void AGameModeSVBattle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
#if 0
	static float Seconds = 0;
	static bool bTraveled = false;
	Seconds += DeltaSeconds;

	if (!bTraveled && Seconds > 5.0f)
	{
		bTraveled = true;
		FString url = FString::Printf(TEXT("/Game/AI/Test/%s?game=%s%s%s?%s=%d%s"), TEXT("NewMap"), TEXT("FFA"), TEXT("?listen"), TEXT("?bIsLanMatch"), TEXT("Bots"), 4, TEXT("?DemoRec"));
		BeforeSeamlessTravel();
		GetWorld()->ServerTravel(url);
	}
#endif
}

void AGameModeSVBattle::BeforeSeamlessTravel()
{
	UWorld *World = GetWorld();
	if (World)
	{
		for (TActorIterator<AAIController> It(World); It;)
		{
			APlayerStateSV *PS = Cast<APlayerStateSV>(It->PlayerState);
			if (PS && PS->PawnId == 0)
			{
				AAIController *AC = *It;
				AC->UnPossess();
				++It;
				World->DestroyActor(AC);
			}
			else {
				++It;
			}
		}
	}
}

void AGameModeSVBattle::StartPlay()
{
	Super::StartPlay();

	Init();
#if 1
	if (bInit)
	{
		//init position
		UWorld *World = GetWorld();
		if (World)
		{
			for (TActorIterator<AController> It(World); It; ++It)
			{
				APlayerStateSV *PS = Cast<APlayerStateSV>(It->PlayerState);
				if (PS && PS->PawnId)
				{
					FString Portal = FString::Printf(TEXT("PS%d"), PS->PawnId);
					AActor* const StartSpot = FindPlayerStart(*It, Portal);
					if (StartSpot != nullptr)
					{
						FRotator InitialControllerRot = StartSpot->GetActorRotation();
						//InitialControllerRot.Roll = 0.f;
						It->SetInitialLocationAndRotation(StartSpot->GetActorLocation(), InitialControllerRot);
						It->StartSpot = StartSpot;

						APawn *Pawn = It->GetPawn();
						if (Pawn == nullptr) {
							Pawn = ChoosePawnById(PS->PawnId);
						}

						if (Pawn)
						{
							Pawn->SetActorRotation(StartSpot->GetActorRotation());
							Pawn->SetActorLocation(StartSpot->GetActorLocation());
						}
					}
				}
			}
		}
	}
#endif
}

void AGameModeSVBattle::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	ActorList.Append(Pawns);

	UWorld *World = GetWorld();
	if (World)
	{
		for (TActorIterator<AMotionControllerProp> It(World); It; ++It) 
		{
			/*for (auto ItMap = It->Props.CreateIterator(); ItMap; ++ItMap) {
				ActorList.Add(ItMap->Value);
			}*/
			ActorList.Add(It->CurrGripTarget);
			It->SeamlessTravelCleanUp();
			ActorList.Add(*It);
		}
	}
}

void AGameModeSVBattle::Init(bool bStartUp)
{
	if (bInit)
		return;

	bInit = true;

	UWorld *World = GetWorld();
	if (World)
	{
		Pawns.Empty();
		AIControllers.Empty();

		for (TActorIterator<AMajorCharacter> It(World); It; ++It)
		{
			if (It->PawnId) {
				Pawns.Add(*It);
			}
		}

		//create character
		if (Pawns.Num() <= 0)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				APlayerStartSV *StartSpot = Cast<APlayerStartSV>(*It);
				if (StartSpot && StartSpot->PlayerStartTag != "")
				{
					AMajorCharacter *Pawn = GetWorld()->SpawnActor<AMajorCharacter>(StartSpot->PawnClass ? StartSpot->PawnClass : AMajorCharacter::StaticClass(), SpawnInfo);
					if (Pawn)
					{
						Pawn->PawnId = FCString::Atoi(*(StartSpot->PlayerStartTag.ToString().Mid(2, 1)));
						Pawn->SetLook(StartSpot->LookId);
						Pawn->JobId = StartSpot->JobId;
						Pawn->TeamID = 0;
						Pawn->AIExtra = StartSpot->AIExtra;
						Pawn->AIControllerClass = StartSpot->AIExtra.bBuildSubPath ? AAIControllerSV::StaticClass() : AAIControllerCrowd::StaticClass();
						Pawn->SpawnDefaultController();
						Pawn->Init(StartSpot->GetActorLocation(), StartSpot->GetActorRotation());

						if (Pawn->Controller)
						{
							FRotator InitialControllerRot = StartSpot->GetActorRotation();
							//InitialControllerRot.Roll = 0.f;
							Pawn->Controller->SetInitialLocationAndRotation(StartSpot->GetActorLocation(), InitialControllerRot);
							Pawn->Controller->StartSpot = StartSpot;
						}

						Pawn->SetActorRotation(StartSpot->GetActorRotation());
						Pawn->SetActorLocation(StartSpot->GetActorLocation());

						Pawns.Add(Pawn);
					}
				}
			}
		}

		//init pc pawnid and possess pawn
		if (!bStartUp)
		{
			for (TActorIterator<APlayerController> It(World); It; ++It)
			{
				AMajorCharacter *Pawn = Cast<AMajorCharacter>(It->GetPawn());
				if (Pawn == nullptr)
				{
					APlayerStateSV *PS = Cast<APlayerStateSV>(It->PlayerState);
					if (PS && PS->PawnId) {
						Pawn = Cast<AMajorCharacter>(ChoosePawnById(PS->PawnId));
					}

					if (Pawn == nullptr) {
						Pawn = Cast<AMajorCharacter>(ChoosePawnNoPlayerController());
					}

					if (Pawn)
					{
						It->Possess(Pawn);

						if (PS) {
							PS->PawnId = Pawn->PawnId;
						}
					}
				}
			}
		}

		//init ac pawnid and possess pawn
		for (TActorIterator<AAIControllerSV> It(World); It; ++It)
		{
			APlayerStateSV *PS = Cast<APlayerStateSV>(It->PlayerState);
			if (PS)
			{
				if (PS->PawnId == 0)
				{
					AMajorCharacter *ResultPawn = Cast<AMajorCharacter>(It->GetPawn());
					if (ResultPawn) {
						PS->PawnId = ResultPawn->PawnId;
					}
				}

				if (PS->PawnId)
				{
					AMajorCharacter *ResultPawn = Cast<AMajorCharacter>(It->GetPawn());
					if (ResultPawn == nullptr)
					{
						APawn *Pawn = ChoosePawnById(PS->PawnId);
						if (Pawn) {
							It->Possess(Pawn);
						}
					}
					else if (It->bSeamlessTravel) {
						It->Init();
					}
				}
			}

			AIControllers.Add(*It);
		}
	}

	InitTargetPoint();
}

void AGameModeSVBattle::InitTargetPoint()
{
	TargetPoints.Reset();
	MasterPathPoints.Reset();

	UWorld *World = GetWorld();
	if (World)
	{
		for (TActorIterator<ATargetPointSV> It(World); It; ++It)
		{
			if (It->TargetPointTag != "") {
				TargetPoints.Add(It->TargetPointTag, *It);
			}
		}

		for (TActorIterator<ALevelConfigActor> It(World); It; ++It)
		{
			for (int i = 0; i < It->MasterPathName.Num(); ++i)
			{
				ATargetPointSV **targetPointPtr = TargetPoints.Find(It->MasterPathName[i]);
				if (targetPointPtr) {
					MasterPathPoints.Add((*targetPointPtr)->GetActorLocation());
				}
			}
		}
	}
}

void AGameModeSVBattle::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);
	
	Init(true);
	
	APlayerControllerSV *PC = Cast<APlayerControllerSV>(newPlayer);
	if (PC)
	{
		APawn *Pawn = nullptr;

		APlayerStateSV *PS = Cast<APlayerStateSV>(PC->PlayerState);
		if (PS && PS->PawnId) {
			Pawn = Cast<AMajorCharacter>(ChoosePawnById(PS->PawnId));
		}

		if (Pawn == nullptr) {
			Pawn = Cast<AMajorCharacter>(ChoosePawnNoPlayerController());
		}

		if (Pawn)
		{
			AAIController *AC = Cast<AAIController>(Pawn->Controller);

			PC->SetInitialLocationAndRotation(Pawn->GetActorLocation(), Pawn->GetActorRotation());
			PC->StartSpot = AC->StartSpot;

			if (AC && AC->PlayerState) {
				AC->PlayerState->SeamlessTravelTo(PC->PlayerState);
			}

			PC->Possess(Pawn);
		}
	}
}

void AGameModeSVBattle::PreLogout(AController* Exiting)
{
	APlayerStateSV *PS = Cast<APlayerStateSV>(Exiting->PlayerState);
	if (PS)
	{
		AAIController *AC = ChooseAIControllerById(PS->PawnId);
		if (AC)
		{
			if (AC->PlayerState) {
				Exiting->PlayerState->SeamlessTravelTo(AC->PlayerState);
			}

			AC->Possess(Exiting->GetPawn());
		}
	}
}

AAIController *AGameModeSVBattle::ChooseAIControllerById(int PawnId)
{
	for (int i = 0; i < AIControllers.Num(); ++i)
	{
		APlayerStateSV *PS = Cast<APlayerStateSV>(AIControllers[i]->PlayerState);
		if (PS && AIControllers[i]->GetPawn() == nullptr && PS->PawnId == PawnId) {
			return Cast<AAIController>(AIControllers[i]);
		}
	}

	return nullptr;
}

APawn *AGameModeSVBattle::ChoosePawnById(int PawnId)
{
	for (int i = 0; i < Pawns.Num(); ++i)
	{
		AMajorCharacter *Pawn = Cast<AMajorCharacter>(Pawns[i]);
		if (Pawn->PawnId == PawnId && (Pawn->Controller == nullptr || !Pawn->Controller->IsPlayerController())) {
			return Pawns[i];
		}
	}

	return nullptr;
}

APawn *AGameModeSVBattle::ChoosePawnNoPlayerController()
{
	for (int i = 0; i < Pawns.Num(); ++i)
	{
		AMajorCharacter *Pawn = Cast<AMajorCharacter>(Pawns[i]);
		if (Pawn->Controller == nullptr || !Pawn->Controller->IsPlayerController()) {
			return Pawns[i];
		}
	}

	return nullptr;
}

AGameModeSVBattle::TConstIterator AGameModeSVBattle::CreatePawnConstIterator()
{
	return Pawns.CreateConstIterator();
}
