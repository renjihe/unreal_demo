// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "SpreadRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


SpreadRes& SpreadRes::Get()
{
	static SpreadRes ins;
	return ins;
}

SpreadRes::SpreadRes()
{
	SpreadDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Spread"));
	SpreadDataTable->AddToRoot();

	FSpreadRow *Row = SpreadDataTable->FindRow<FSpreadRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FSpreadRow*> AllRow;
		SpreadDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FSpreadRow& SpreadRes::GetSpread(int RecoilId)
{
	FSpreadRow *spread = SpreadDataTable->FindRow<FSpreadRow>(FName(*FString::FromInt(RecoilId)), FString(""));
	if (nullptr == spread) {
		spread = &DefaultRow;
	}
	
	return *spread;
}