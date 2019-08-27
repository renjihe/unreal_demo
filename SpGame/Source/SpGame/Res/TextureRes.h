// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "TextureRes.generated.h"

USTRUCT(BlueprintType)
struct FTextureRow: public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UTexture2D> texture;
};

class SPGAME_API FTextureRes
{
	//UObjectLibrary *LibAnimClass;
	//UObjectLibrary *LibMesh;

	FTextureRow	DefaultRow;

	UDataTable	*TextureDataTable;

	FTextureRes();
public:		
	static FTextureRes& Get();

	FTextureRow& GetTexture(int textureId);
};
