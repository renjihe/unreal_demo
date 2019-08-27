// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilityInfoRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>

FAbilityInfoRow	AbilityInfoRes::DefaultRow;

AbilityInfoRes& AbilityInfoRes::Get()
{
	static AbilityInfoRes ins;
	return ins;
}

AbilityInfoRes::AbilityInfoRes()
{
	AbilityDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/AbilityInfo"));
	AbilityDataTable->AddToRoot();

/*	FAbilityInfoRow *Row = AbilityDataTable->FindRow<FAbilityInfoRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FAbilityInfoRow*> AllRow;
		AbilityDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}*/
}

bool AbilityInfoRes::IsExist(int AbilityID)
{
	FAbilityInfoRow *AbilityInfo = AbilityDataTable->FindRow<FAbilityInfoRow>(FName(*FString::FromInt(AbilityID)), FString(""));
	if (nullptr == AbilityInfo) {
		return false;
	}

	return true;
}

FAbilityInfoRow& AbilityInfoRes::GetAbilityinfo(int AbilityID)
{
	FAbilityInfoRow *AbilityInfo = AbilityDataTable->FindRow<FAbilityInfoRow>(FName(*FString::FromInt(AbilityID)), FString(""));
	if (nullptr == AbilityInfo) {
		AbilityInfo = &DefaultRow;
	}
	
	return *AbilityInfo;
}