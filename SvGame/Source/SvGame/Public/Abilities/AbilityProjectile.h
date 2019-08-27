// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/CharacterSV.h"
#include "Character/MajorCharacter.h"
#include "Common/GameData.h"
#include "AbilityProjectile.generated.h"
class UProjectileMovementComponent;
//class AMajorCharacter;
/**
 * 
 */
UCLASS()
class SVGAME_API AAbilityProjectile : public ACharacterSV
{
	GENERATED_BODY()
	
	AAbilityProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
private:
	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

public:
	void ActivateProjectile(EAbilityProjectile::Type projectiletype);
	void SetProjectileVelocity(FVector projectileVelocity, float time = 0.0f);
	void ActivationTraceMode(ACharacterSV* target);
	UFUNCTION()
	void OnProjectileStop(const FHitResult& hit);

	virtual void AbilityEnded(int abilityID) override;

protected:
	void AbilityDelayTouchOff();
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
protected:
	FTimerHandle DelayTouchOffTimer;

	float DelayTouchOff;
	EAbilityProjectile::Type ProjectileType;
};
