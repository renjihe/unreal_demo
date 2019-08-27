// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CameraComponentSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API UCameraComponentSV : public UCameraComponent
{
	GENERATED_UCLASS_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
};
