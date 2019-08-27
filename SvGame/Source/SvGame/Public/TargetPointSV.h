// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/TargetPoint.h"
#include "TargetPointSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API ATargetPointSV : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TargetPoint")
	FName TargetPointTag;
};
