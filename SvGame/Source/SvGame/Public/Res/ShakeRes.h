// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "ShakeRes.generated.h"
/**
* FShakeRow
*/
USTRUCT(BlueprintType)
struct FShakeRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RecoveryInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxTotalPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxTotalPitchVariance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxTotalYaw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float XPush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxTotalXPushSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* ShakeCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;
};

class SVGAME_API ShakeRes
{
	FShakeRow	DefaultRow;
	UDataTable*		ShakeDataTable;

	ShakeRes();
public:		
	static ShakeRes& Get();

	FShakeRow& GetShake(int ShakeID);
};
