// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GlobalConfigRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


FGlobalConfigRes& FGlobalConfigRes::Get()
{
	static FGlobalConfigRes ins;
	return ins;
}

FGlobalConfigRes::FGlobalConfigRes()
{
	ConfigDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Global"));
	ConfigDataTable->AddToRoot();

	FGlobalConfigRow *Row = ConfigDataTable->FindRow<FGlobalConfigRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else
	{
		TArray<FGlobalConfigRow*> AllRow;
		ConfigDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

const FGlobalConfigRow& FGlobalConfigRes::GetConfig(FName ConfigId)
{
	FGlobalConfigRow *Config = ConfigDataTable->FindRow<FGlobalConfigRow>(ConfigId, FString(""));
	if (nullptr == Config) {
		Config = &DefaultRow;
	}

	return *Config;
}
