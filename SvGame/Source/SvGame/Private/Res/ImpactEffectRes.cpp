// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "ImpactEffectRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


ImpactEffectRes& ImpactEffectRes::Get()
{
	static ImpactEffectRes ins;
	return ins;
}

ImpactEffectRes::ImpactEffectRes()
{
	ImpactEffectDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Sound"));
	ImpactEffectDataTable->AddToRoot();

	FImpactEffectRow *Row = ImpactEffectDataTable->FindRow<FImpactEffectRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FImpactEffectRow*> AllRow;
		ImpactEffectDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FImpactEffectRow& ImpactEffectRes::GetImpactEffect(int ImpactEffectID)
{
	FImpactEffectRow *ImpactEffect = ImpactEffectDataTable->FindRow<FImpactEffectRow>(FName(*FString::FromInt(ImpactEffectID)), FString(""));
	if (nullptr == ImpactEffect) {
		ImpactEffect = &DefaultRow;
	}
	
	return *ImpactEffect;
}