// Fill out your copyright notice in the Description page of Project Settings.

#include "SprayColorRes.h"
#include "SpGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

FSprayColorRes& FSprayColorRes::Get()
{
	static FSprayColorRes ins;
	return ins;
}

FSprayColorRes::FSprayColorRes()	
{
	ColorDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/SprayColor"));
	ColorDataTable->AddToRoot();

	FSprayColorRow *row = ColorDataTable->FindRow<FSprayColorRow>(FName("0"), FString(""));
	if (row) {
		DefaultRow = *row;
	}
	else 
	{
		TArray<FSprayColorRow*> allRow;
		ColorDataTable->GetAllRows(FString(""), allRow);
		if (allRow.Num()) {
			DefaultRow = *allRow[0];
		}
	}
}

FSprayColorRow& FSprayColorRes::GetColor(int colorId)
{
	if (colorId < 0) {
		colorId = UKismetMathLibrary::RandomIntegerInRange(0, ColorDataTable->RowMap.Num());
	}

	FSprayColorRow *Color = ColorDataTable->FindRow<FSprayColorRow>(FName(*FString::FromInt(colorId)), FString("FSprayColorRes::GetColor"));
	if (nullptr == Color) {
		Color = &DefaultRow;
	}
	
	return *Color;
}

int FSprayColorRes::GetColorCount()
{
	return ColorDataTable->GetRowNames().Num();
}