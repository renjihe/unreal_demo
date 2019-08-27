// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEdEngine.h"
#include "SpUnrealEdEngine.generated.h"

/**
 * 
 */
UCLASS()
class SPGAMEED_API USpUnrealEdEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;
	
};
