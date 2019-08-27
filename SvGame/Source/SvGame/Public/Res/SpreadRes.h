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

	//ɢ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* SpreadCurve;

	//��ɢ�ٶ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpreadIncrement;

	//�ӳٻָ��ٶ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LaggyRecoverTime;

	//ɢ���ָ��ٶ�
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
