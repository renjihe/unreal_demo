// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "SoundRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


SoundRes& SoundRes::Get()
{
	static SoundRes ins;
	return ins;
}

SoundRes::SoundRes()
{
	SoundDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Sound"));
	SoundDataTable->AddToRoot();

	FSoundRow *Row = SoundDataTable->FindRow<FSoundRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FSoundRow*> AllRow;
		SoundDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FSoundRow& SoundRes::GetSound(int SoundID)
{
	FSoundRow *Sound = SoundDataTable->FindRow<FSoundRow>(FName(*FString::FromInt(SoundID)), FString(""));
	if (nullptr == Sound) {
		Sound = &DefaultRow;
	}
	
	return *Sound;
}