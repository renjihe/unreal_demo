// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GlobalConfigRes.generated.h"

USTRUCT(BlueprintType)
struct FGlobalConfigRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int> IntValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> FloatValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Desc;
};

class SPGAME_API FGlobalConfigRes
{
	FGlobalConfigRow	DefaultRow;
	UDataTable *ConfigDataTable;

	FGlobalConfigRes();
public:
	static FGlobalConfigRes& Get();

	const FGlobalConfigRow& GetConfig(FName ConfigId);
};
