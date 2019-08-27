// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Effect/HitImpactEffect.h"
#include "ImpactEffectRes.generated.h"
/**
* FSoundRow
*/
USTRUCT(BlueprintType)
struct FImpactEffectRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AHitImpactEffect> ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;

};

class SVGAME_API ImpactEffectRes
{
	FImpactEffectRow DefaultRow;
	UDataTable* ImpactEffectDataTable;

	ImpactEffectRes();
public:		
	static ImpactEffectRes& Get();

	FImpactEffectRow& GetImpactEffect(int ImpactEffectID);
};
