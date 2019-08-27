// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "SpreadRes.generated.h"
/**
* FShakeRow
*/
USTRUCT(BlueprintType)
struct FSpreadRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	//散布曲线
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* SpreadCurve;

	//扩散速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpreadIncrement;

	//延迟恢复速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LaggyRecoverTime;

	//散布恢复速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpreadRecover;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;
};

class SVGAME_API SpreadRes
{
	FSpreadRow	DefaultRow;
	UDataTable*		SpreadDataTable;

	SpreadRes();
public:		
	static SpreadRes& Get();

	FSpreadRow& GetSpread(int SpreadId);
};
