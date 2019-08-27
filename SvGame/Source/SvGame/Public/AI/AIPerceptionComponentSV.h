// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Perception/AIPerceptionComponent.h"
#include "AIPerceptionComponentSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API UAIPerceptionComponentSV : public UAIPerceptionComponent
{
	GENERATED_BODY()
	
public:
	void RemoveDeadData();
	
};
