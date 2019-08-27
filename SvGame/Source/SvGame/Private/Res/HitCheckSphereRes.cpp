// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "HitCheckSphereRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


HitCheckSphereRes& HitCheckSphereRes::Get()
{
	static HitCheckSphereRes ins;
	return ins;
}

HitCheckSphereRes::HitCheckSphereRes()
{
	HitCheckSphereDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/HitCheckSphere"));
	HitCheckSphereDataTable->AddToRoot();

	FHitCheckSphereRow *Row = HitCheckSphereDataTable->FindRow<FHitCheckSphereRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FHitCheckSphereRow*> AllRow;
		HitCheckSphereDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FHitCheckSphereRow& HitCheckSphereRes::GetHitCheckSphere(int HitCheckSphereID)
{
	FHitCheckSphereRow *HitCheckSphere = HitCheckSphereDataTable->FindRow<FHitCheckSphereRow>(FName(*FString::FromInt(HitCheckSphereID)), FString(""));
	if (nullptr == HitCheckSphere) {
		HitCheckSphere = &DefaultRow;
	}
	
	return *HitCheckSphere;
}