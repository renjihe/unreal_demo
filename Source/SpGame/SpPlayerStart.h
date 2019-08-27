// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SpPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API ASpPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	int Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	int PawnId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	int Frame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	int RebirthTime;
};
