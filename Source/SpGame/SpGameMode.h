// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SpGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ASpGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void OnInited();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable)
	static void Test(UObject* WorldContextObject);

private:
	bool bInited;
};
