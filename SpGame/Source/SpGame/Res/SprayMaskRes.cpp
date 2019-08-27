// Fill out your copyright notice in the Description page of Project Settings.

#include "SprayMaskRes.h"
#include "SpGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

FSprayMaskRes& FSprayMaskRes::Get()
{
	static FSprayMaskRes ins;
	return ins;
}

FSprayMaskRes::FSprayMaskRes()	
{
	MaskDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/SprayMask"));
	MaskDataTable->AddToRoot();

	FSprayMaskRow *row = MaskDataTable->FindRow<FSprayMaskRow>(FName("0"), FString(""));
	if (row) {
		DefaultRow = *row;
	}
	else 
	{
		TArray<FSprayMaskRow*> allRow;
		MaskDataTable->GetAllRows(FString(""), allRow);
		if (allRow.Num()) {
			DefaultRow = *allRow[0];
		}
	}
}

FSprayMaskRow& FSprayMaskRes::GetMask(int maskId)
{
	if (maskId < 0) {
		maskId = UKismetMathLibrary::RandomIntegerInRange(0, MaskDataTable->RowMap.Num());
	}

	FSprayMaskRow *mask = MaskDataTable->FindRow<FSprayMaskRow>(FName(*FString::FromInt(maskId)), FString("FSprayMaskRes::GetMask"));
	if (nullptr == mask) {
		mask = &DefaultRow;
	}
	
	return *mask;
}

int FSprayMaskRes::GetMaskCount()
{
	return MaskDataTable->GetRowNames().Num();
}