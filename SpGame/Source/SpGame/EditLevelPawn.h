// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/DefaultPawn.h"
#include "EditLevelPawn.generated.h"

/**
 * 
 */
UCLASS()
class AEditLevelPawn : public ADefaultPawn
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY()
	USceneCaptureComponent2D *SceneCapture;

};
