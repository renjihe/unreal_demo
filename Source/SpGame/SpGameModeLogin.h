// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpGameMode.h"
#include "SpGameModeLogin.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API ASpGameModeLogin : public ASpGameMode
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void OnInited() override;
};
