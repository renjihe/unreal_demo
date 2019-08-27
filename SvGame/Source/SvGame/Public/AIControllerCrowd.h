// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIControllerSV.h"
#include "AIControllerCrowd.generated.h"
/**
*
*/
UCLASS()
class SVGAME_API AAIControllerCrowd : public AAIControllerSV
{
	GENERATED_BODY()

public:
	AAIControllerCrowd(const FObjectInitializer& ObjectInitializer);
};
