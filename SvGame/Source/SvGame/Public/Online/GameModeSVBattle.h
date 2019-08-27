// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Online/GameModeSV.h"
#include "GameModeSVBattle.generated.h"

class APawn;
class AAIController;
class ATargetPointSV;
/**
 * 
 */
UCLASS()
class SVGAME_API AGameModeSVBattle : public AGameModeSV
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void StartPlay() override;
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;

	//virtual void BeginPlay() override;
	virtual void Init(bool bStartUp = false);
	virtual void InitTargetPoint();

	virtual void PostLogin(APlayerController* newPlayer) override;

	virtual void PreLogout(AController* Exiting) override;
	virtual void BeforeSeamlessTravel() override;
	//virtual void Logout(AController* Exiting) override;
	typedef TArray<APawn*>::TConstIterator TConstIterator;
	TConstIterator CreatePawnConstIterator();
private:
	bool bInit;

	AAIController *ChooseAIControllerById(int PawnId);
	APawn *ChoosePawnById(int PawnId);
	APawn *ChoosePawnNoPlayerController();

	TArray<AController*> AIControllers;
	TArray<APawn*> Pawns;

public:
	TMap<FName, ATargetPointSV*> TargetPoints;
	TArray<FVector> MasterPathPoints;
};
