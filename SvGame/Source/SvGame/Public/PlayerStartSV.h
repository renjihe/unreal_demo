// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "PlayerStartSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API APlayerStartSV : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, Category = Barracks)
	int LookId;

	UPROPERTY(EditAnywhere, Category = Barracks)
	int JobId;

	UPROPERTY(EditAnywhere, Category = Barracks)
	FAIExtra AIExtra;
};
