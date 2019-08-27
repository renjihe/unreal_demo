// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameModeSV.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeSVMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API AGameModeSVMainMenu : public AGameModeSV
{
	GENERATED_UCLASS_BODY()
	
public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;

	/*virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
	
	
	virtual void PostLogin(APlayerController* newPlayer) override;
	virtual void HandleMatchHasStarted() override;*/
};
