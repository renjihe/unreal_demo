// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "SprayColorRes.generated.h"

/**
* FSprayColorRow 
*/
USTRUCT(BlueprintType)
struct FSprayColorRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FColor AtkerColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FColor DeferColor;

	FSprayColorRow() 
	{
		AtkerColor = FColor(255, 0, 128, 255);
		DeferColor = FColor(0, 255, 255, 255);
	}
};

class SPGAME_API FSprayColorRes
{
	FSprayColorRow	DefaultRow;
	UDataTable	*ColorDataTable;

	FSprayColorRes();
public:		
	static FSprayColorRes& Get();

	FSprayColorRow& GetColor(int colorId);
	int GetColorCount();
};
