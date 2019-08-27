// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "SpGameEngine.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API USpGameEngine : public UGameEngine
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;
	
};
