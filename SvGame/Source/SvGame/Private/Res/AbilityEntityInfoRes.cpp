// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilityEntityInfoRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


AbilityEntityInfoRes& AbilityEntityInfoRes::Get()
{
	static AbilityEntityInfoRes ins;
	return ins;
}

AbilityEntityInfoRes::AbilityEntityInfoRes()
{
	AbilityEntityInfoTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/AbilityEntityInfo"));
	AbilityEntityInfoTable->AddToRoot();

	FAbilityEntityInfoRow *Row = AbilityEntityInfoTable->FindRow<FAbilityEntityInfoRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FAbilityEntityInfoRow*> AllRow;
		AbilityEntityInfoTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FAbilityEntityInfoRow& AbilityEntityInfoRes::GetAbilityEntityInfo(int AbilityEntityInfoID)
{
	FAbilityEntityInfoRow *abilityEntityInfo = AbilityEntityInfoTable->FindRow<FAbilityEntityInfoRow>(FName(*FString::FromInt(AbilityEntityInfoID)), FString(""));
	if (nullptr == abilityEntityInfo) {
		abilityEntityInfo = &DefaultRow;
	}
	
	return *abilityEntityInfo;
}