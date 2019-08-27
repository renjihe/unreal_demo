// Fill out your copyright notice in the Description page of Project Settings.

#include "GlobalConfigRes.h"
#include "SpGame.h"

FGlobalConfigRes& FGlobalConfigRes::Get()
{
	static FGlobalConfigRes ins;
	return ins;
}

FGlobalConfigRes::FGlobalConfigRes()
{
	ConfigDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/Global"));
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
