// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "AvatarRes.generated.h"

/**
* FSprayColorRow 
*/
USTRUCT(BlueprintType)
struct FAvatarRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UClass *Avatar;
};

class SPGAME_API FAvatarRes
{
	FAvatarRow	DefaultRow;
	UDataTable	*DataTable;

	FAvatarRes();
public:		
	static FAvatarRes& Get();

	FAvatarRow& GetAvatar(int avatarId);
};
