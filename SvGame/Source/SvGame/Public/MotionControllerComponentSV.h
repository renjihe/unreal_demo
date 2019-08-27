// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MotionControllerComponent.h"
#include "MotionControllerComponentSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API UMotionControllerComponentSV : public UMotionControllerComponent
{
	GENERATED_UCLASS_BODY()


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
