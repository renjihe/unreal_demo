// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterSV.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "MobCharacter.generated.h"

class UPhysicalAnimationComponent;


UCLASS()
class SVGAME_API AMobCharacter : public ACharacterSV
{
	GENERATED_UCLASS_BODY()
public:

	virtual void NotifyDead(const FHitResult &hit, EAttackType::Type attackType = EAttackType::UnDefined) override;

	void DeadAction();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;


};
