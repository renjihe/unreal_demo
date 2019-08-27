// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "BTRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


FBTRes& FBTRes::Get()
{
	static FBTRes ins;
	return ins;
}

FBTRes::FBTRes()
{
	BTDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/BT"));
	BTDataTable->AddToRoot();

	FBTRow *Row = BTDataTable->FindRow<FBTRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FBTRow*> AllRow;
		BTDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FBTRow& FBTRes::GetBT(int BTId)
{
	FBTRow *BT = BTDataTable->FindRow<FBTRow>(FName(*FString::FromInt(BTId)), FString(""));
	if (nullptr == BT) {
		BT = &DefaultRow;
	}
	
	return *BT;
}

bool FBTRes::CreateBT(int BTId, UBehaviorTree *&BTAsset)
{
	FBTRow &Row = GetBT(BTId);

	Row.BTAsset.LoadSynchronous();
	BTAsset = Row.BTAsset.Get();

	return true;
}