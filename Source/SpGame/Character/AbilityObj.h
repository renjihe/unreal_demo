// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/SpCharacter.h"
#include "Common/GameData.h"
#include "AbilityObj.generated.h"

class FEntityComponent;
//class AMajorCharacter;
/**
 * 
 */
UCLASS()
class SPGAME_API AAbilityObj : public AActor 
{
	GENERATED_UCLASS_BODY()

private:
	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

public:
	unsigned EntityId;

	FEntityComponent *GetEntityComponent() {
		return EntityComponent.Get();
	}

	TSharedPtr<FEntityComponent> EntityComponent;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
public:
	int Faction;
};
