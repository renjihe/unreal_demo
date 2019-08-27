// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Common/GameData.h"
#include "JobRes.generated.h"

#define Invalid_Job -1

/**
* FLookRow
*/
USTRUCT(BlueprintType)
struct FJobRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int BTId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TEnumAsByte<ESpeedType::Type>, float> SpeedTypeGain;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TEnumAsByte<EAttribType::Type>, int> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TEnumAsByte<EAbilityBar::Type>, int> AbilityList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;
};

class SVGAME_API FJobRes
{
	FJobRow	DefaultRow;
	UDataTable *JobDataTable;

	FJobRes();
public:		
	static FJobRes& Get();

	const FJobRow& GetJob(int JobId);
};
