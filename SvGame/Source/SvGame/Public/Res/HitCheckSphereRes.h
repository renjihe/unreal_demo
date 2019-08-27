// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "HitCheckSphereRes.generated.h"

USTRUCT(BlueprintType)
struct FHitSphereInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	FName ParentSocket;

	UPROPERTY(EditDefaultsOnly)
	float Radius;

	UPROPERTY(EditDefaultsOnly)
	float Delay;

	UPROPERTY(EditDefaultsOnly)
	float Duration;
};

/**
* FHitSphereCheckRow
*/
USTRUCT(BlueprintType)
struct FHitCheckSphereRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	//是否可以产生二次伤害
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool AgainAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool DrawDebug;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FHitSphereInfo> HitSphereInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;
};

class SVGAME_API HitCheckSphereRes
{
	FHitCheckSphereRow	DefaultRow;
	UDataTable* HitCheckSphereDataTable;

	HitCheckSphereRes();
public:		
	static HitCheckSphereRes& Get();

	FHitCheckSphereRow& GetHitCheckSphere(int HitCheckSphereID);
};
