// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SpGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API USpGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void Init();
	virtual void Shutdown();
private:
	TSharedPtr<class FSpInputPreprocessor> InputProcessor;
};

extern UGameInstance *GGameInstance;