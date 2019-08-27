// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "ShakeRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


ShakeRes& ShakeRes::Get()
{
	static ShakeRes ins;
	return ins;
}

ShakeRes::ShakeRes()
{
	ShakeDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Shake"));
	ShakeDataTable->AddToRoot();

	FShakeRow *Row = ShakeDataTable->FindRow<FShakeRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FShakeRow*> AllRow;
		ShakeDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FShakeRow& ShakeRes::GetShake(int ShakeID)
{
	FShakeRow *shake = ShakeDataTable->FindRow<FShakeRow>(FName(*FString::FromInt(ShakeID)), FString(""));
	if (nullptr == shake) {
		shake = &DefaultRow;
	}
	
	return *shake;
}