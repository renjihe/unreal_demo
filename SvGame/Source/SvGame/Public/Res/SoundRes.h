// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "SoundRes.generated.h"
/**
* FSoundRow
*/
USTRUCT(BlueprintType)
struct FSoundRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Volume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FalloffDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;


	FSoundRow()
	{
		Volume = 1.0f;
		Pitch = 1.0f;
		Radius = 400.0f;
		FalloffDistance = 3600.0f;
	}
};

class SVGAME_API SoundRes
{
	FSoundRow	DefaultRow;
	UDataTable* SoundDataTable;

	SoundRes();
public:		
	static SoundRes& Get();

	FSoundRow& GetSound(int SoundID);
};
