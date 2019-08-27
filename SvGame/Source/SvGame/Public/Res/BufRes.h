// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "BufRes.generated.h"

UENUM(BlueprintType)
namespace EBufFuncType
{
	enum Type
	{
		NONE,
		ATTRIB,
		ATTRIB_PERCENT,
		COSPLAY,
		POST_PROCESS,
		EFFECT,
		PERIODIC,
		EXTREMIS,
	};
}

USTRUCT(BlueprintType)
struct FBufFuncRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EBufFuncType::Type> Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int> Args;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> StringArgs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVector> VectorArgs;
};

USTRUCT(BlueprintType)
struct FBufRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDebuf;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBufFuncRow> Funcs;
};

class SVGAME_API FBufRes
{
	FBufRow	DefaultRow;
	UDataTable *BufDataTable;

	FBufRes();
public:
	static FBufRes& Get();

	const FBufRow& GetBuf(int BufId);
};
