// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SoundRes.h"
#include "ParticleRes.h"
#include "ShakeRes.h"
#include "AbilityInfoRes.h"
#include "SpreadRes.h"
#include "BTRes.h"
#include "JobRes.h"
#include "LookRes.h"
#include "AbilityEntityInfoRes.h"
//#include "HitTestActorRes.h"
#include "ResLibrary.generated.h"

USTRUCT(BlueprintType)
struct FKCapsulesItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Center;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FQuat Orientation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Length;
};

USTRUCT(BlueprintType)
struct FBodySetupItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FKCapsulesItem> Capsules;
};

UCLASS()
class SVGAME_API UResLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Category = "Res")
	static FSoundRow GetSoundByID(int SoundId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FParticleRow GetParticleByID(int ParticleId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FShakeRow GetGunRecoilByID(int GunRecoilId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FAbilityInfoRow GetAbilityInfoByID(int AbilityId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FSpreadRow GetGunSpreadByID(int GunSpreadId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FBTRow GetBTByID(int BTId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FJobRow GetJobByID(int JobId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FLookRow GetLookByID(int lookId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static FAbilityEntityInfoRow GetAbilityEntityInfoByID(int AbilityEntityInfoId);

	//UFUNCTION(BlueprintCallable, Category = "Res")
	//static FHitTestActorRow GetHitTestActorByID(int HitTestActorId);

	UFUNCTION(BlueprintPure, Category = "Res")
	static UAnimMontage* LoadAnimMontage(const FString &Path);

	UFUNCTION(BlueprintCallable, Category = "Res")
	static void GetBodySetUps(UPhysicsAsset* physiceAsset, TArray<FBodySetupItem> &bodySetups);
};
