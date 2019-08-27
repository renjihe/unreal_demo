// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Res/AbilityInfoRes.h"
#include "Abilities/AbilitiesType.h"

#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySV.generated.h"

class AAbilityProjectile;

/**
 * 
 */
UCLASS(abstract, Blueprintable)
class SVGAME_API UGameplayAbilitySV : public UGameplayAbility
{
	GENERATED_UCLASS_BODY()

	friend class ACharacterSV;

	UPROPERTY(BlueprintReadOnly)
	int32 AbilityID;

	UPROPERTY(BlueprintReadOnly)
	FAbilityInfoRow AbilityInfo;

	UPROPERTY(BlueprintReadOnly)
	mutable FAbilityState AbilityState;

	UPROPERTY(BlueprintReadOnly)
	int32 TeamID;

	UPROPERTY(BlueprintReadOnly)
	FAttribs Attribs;

	UPROPERTY(BlueprintReadOnly)
	int AbilityIndex;

	UPROPERTY(BlueprintReadOnly)
	AActor* Target;

	virtual uint8 GetGameplayTaskDefaultPriority() const {
		return FGameplayTasks::DefaultPriority; 
	}

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void SetAbilityInfo(int newAbilityID, FAbilityInfoRow newAbilityInfo, FAttribs newAttribs, int newTeam, int newAbilityIndex);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void SetAttribs(FAttribs ownerAttribs);

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	int GetAbilityID();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void SetTeam(int newTeamID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Custom")
	void AbilityTick(float Delta);

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Custom")
	bool IsNeedTick();

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Custom")
	void InitAbility();

	UFUNCTION(BlueprintCallable)
	class ACharacterSV* GetCaster() const;

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	int GetIntParam(int index) const {
		return index < AbilityInfo.ParamInts.Num() ? AbilityInfo.ParamInts[index] : 0;
	}

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	FString GetStringParam(int index) const {
		return index < AbilityInfo.ParamStrings.Num() ? AbilityInfo.ParamStrings[index] : "";
	}

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	FAbilityTypeFlags GetAbilityFlags() const {
		return AbilityInfo.AbilityFlags;
	}

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	int GetAbilityValue() const {
		return AbilityInfo.Value;
	}

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
		float GetAbilityRadius() const {
		return AbilityInfo.Radius;
	}

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
		float GetAbilityDuration() const {
		return AbilityInfo.Duration;
	}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Custom")
	void NotifyOnHit(FHitResult hit);
	
	UFUNCTION(BlueprintCallable)
	void HitTestRay(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, float rayLength, int targetCount, EAbilityTarget::Type targetType);

	UFUNCTION(BlueprintCallable)
	void HitTestSphere(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float radius, float duration
		,  int targetCount, EAbilityTarget::Type targetType, EHitTestMode::Mode testMode = EHitTestMode::Always, float periodInterval = 0, bool drawDebug = false);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void Damage(const FHitResult &hitResult, EAttackType::Type abilityType, int damage, int perDamage, bool PlayEffect = true);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AddHP(AActor* targethp, int value);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AddBuff(const FHitResult &hitResult, int BuffID, int Frames, int BuffPeriodicFrames);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void RemoveBuff(const FHitResult &hitResult, int BuffID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void PlayParticle(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation = FVector::ZeroVector, FRotator relativeRotation = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void PlayParticleTrail(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation = FVector::ZeroVector, FRotator relativeRotation = FRotator::ZeroRotator
										 , FName toParamName = NAME_None, AActor *toActor = nullptr, EEffectPosition::Type toPosition = EEffectPosition::Actor, FName toSocket = NAME_None, FVector toOffset = FVector::ZeroVector);
	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void PlaySound(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation = FVector::ZeroVector, FRotator relativeRotation = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void SpawnImpactEffects(const FHitResult& hit, TSubclassOf<AHitImpactEffect> impactEffect);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool TryToConsume();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void CalcShakeOffset();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void GunFireTrace(float radius, int trailParticleID, FName trailTargetParam, TArray<FHitResult> &hits, EAbilityTarget::Type targetType, EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void ReportNoiseEvent(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float Loudness = 1.f, float MaxRange = 0.f, FName Tag = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void PlayForceFeedback(class UForceFeedbackEffect* ForceFeedbackEffect, bool bLooping, FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void BombSphereTrace(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float radius, EAbilityTarget::Type targetType, EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void SpawnProjectile(TSubclassOf<AAbilityProjectile> projectile, EAbilityProjectile::Type ProjectileType, int time = 0, ACharacterSV* homingtarget = nullptr);

public:
	bool FilterHitTarget(AActor *target, EAbilityTarget::Type targetType);

protected:
	TSet<AActor*> HitTestActors;
	TSet<AActor*> HitTargets;

	UFUNCTION(BlueprintImplementableEvent)
	void OnHit(const FHitResult &hitResult);

public:
	void DestroyHitTestActor(class AHitTestActor *hitTestActor);
	bool HitTarget(const FHitResult &hitResult);
};
