// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "SprayMaskRes.generated.h"

/**
* FSprayColorRow
*/
USTRUCT(BlueprintType)
struct FSprayMaskRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D *Mask;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range;

	FSprayMaskRow() {
		Range = 0.8f;
	}
};

class SPGAME_API FSprayMaskRes
{
	FSprayMaskRow	DefaultRow;
	UDataTable	*MaskDataTable;

	FSprayMaskRes();
public:		
	static FSprayMaskRes& Get();

	FSprayMaskRow& GetMask(int maskId);
	int GetMaskCount();
};
