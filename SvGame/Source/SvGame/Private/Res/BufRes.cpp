// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "BufRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


FBufRes& FBufRes::Get()
{
	static FBufRes ins;
	return ins;
}

FBufRes::FBufRes()
{
	BufDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Buf"));
	BufDataTable->AddToRoot();

	FBufRow *Row = BufDataTable->FindRow<FBufRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else
	{
		TArray<FBufRow*> AllRow;
		BufDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

const FBufRow& FBufRes::GetBuf(int BufId)
{
	FBufRow *Buf = BufDataTable->FindRow<FBufRow>(FName(*FString::FromInt(BufId)), FString(""));
	if (nullptr == Buf) {
		Buf = &DefaultRow;
	}

	return *Buf;
}
