// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "BTRes.generated.h"
/**
* FLookRow
*/
USTRUCT(BlueprintType)
struct FBTRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UBehaviorTree> BTAsset;
};

class SVGAME_API FBTRes
{
	FBTRow	DefaultRow;
	UDataTable *BTDataTable;

	FBTRes();
public:		
	static FBTRes& Get();

	FBTRow& GetBT(int BTId);

	bool CreateBT(int BTId, UBehaviorTree *&BTAsset);
};
