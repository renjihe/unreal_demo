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

	//ÿ�β���Ч������(�ӵ�������������)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PerNum;

	//the skill is limit by other condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Infinite;

	//The quantity of this things in the group
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInGroup;

	//ʹ��������InfiniteΪtrueʱ�˲�����Ч��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInPak;

	//��ɢ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SpreadID;

	//�ζ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ShakeID;

	//������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TraceSocket;

	//���Ŀ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius;

	//Ŀ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TargetCount;

	//��λ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TEnumAsByte<EPhysicalSurface>, float> PartAdjust;

	//˥��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damp;

	//˥����Χ
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DampRadius;

	//�Ƿ�������
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
