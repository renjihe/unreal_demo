// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpGameMode.h"
#include <common/battledef.h>
#include "SpGameModeBattle.generated.h"

namespace summer {
	struct BattleRealCmdData;
}
class ASpCharacter;
/**
 * 
 */
UCLASS()
class AGameModeBattle : public ASpGameMode
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaSeconds);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void OnInited() override;

	virtual void StartPlay() override;
	virtual void Init(bool bStartUp = false);

	typedef TArray<ASpCharacter*>::TConstIterator TConstIterator;
	TConstIterator CreatePawnConstIterator();

	void DoRealCmd(const summer::BattleRealCmdData *cmd);

	LevelConfig LevelCfg;
	FVector AtkerColor, DeferColor;
	float 	AtkerScore, DeferScore;
private:
	bool bInit;
	TArray<ASpCharacter*> Pawns;
};
