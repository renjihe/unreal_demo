// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "GameModeSV.generated.h"

/*USTRUCT()
struct FDecalItem
{
	GENERATED_USTRUCT_BODY()

	UDecalComponent* Comp;
	float AutoDestroyTime;
};
*/
/**
 * 
 */
UCLASS()
class SVGAME_API AGameModeSV : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostLogin(APlayerController* newPlayer) override;
	virtual void HandleMatchHasStarted() override;

	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList);

	virtual void PreLogout(AController* Exiting) {};

	virtual void BeforeSeamlessTravel() {};
	void ServerTestCmd(const FString &Cmd, const FString &Arg);

	/*UPROPERTY(EditDefaultsOnly)
	int MaxDecal;

	TArray<FDecalItem> DecalArr;

	void AddDecalItem(FDecalItem decal);*/
};
