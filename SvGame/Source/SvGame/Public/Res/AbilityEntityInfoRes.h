// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "AbilityEntityInfoRes.generated.h"
/**
* FAbilityEntityInfoRow
*/
USTRUCT(BlueprintType)
struct FAbilityEntityInfoRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	//每次产生效果数量(子弹：单发弹丸数)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PerNum;

	//the skill is limit by other condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Infinite;

	//The quantity of this things in the group
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInGroup;

	//使用总数（Infinite为true时此参数无效）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInPak;

	//扩散参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SpreadID;

	//晃动参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ShakeID;

	//检测骨骼
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TraceSocket;

	//检测的宽度
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius;

	//目标数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TargetCount;

	//部位修正
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TEnumAsByte<EPhysicalSurface>, float> PartAdjust;

	//衰减
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damp;

	//衰减范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DampRadius;

	//是否是曲线
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplineVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShowSplineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;

	FAbilityEntityInfoRow()
	{
		PerNum = 1;
		Infinite = false;
		MaxInGroup = 10;
		MaxInPak = 100;
		SpreadID = 0;
		ShakeID = 0;
		TargetCount = 1;
		IsSpline = false;
	}
};

class SVGAME_API AbilityEntityInfoRes
{
	FAbilityEntityInfoRow	DefaultRow;
	UDataTable* AbilityEntityInfoTable;

	AbilityEntityInfoRes();
public:		
	static AbilityEntityInfoRes& Get();

	FAbilityEntityInfoRow& GetAbilityEntityInfo(int AbilityEntityInfoID);
};
