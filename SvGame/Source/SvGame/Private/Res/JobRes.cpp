// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "JobRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


FJobRes& FJobRes::Get()
{
	static FJobRes ins;
	return ins;
}

FJobRes::FJobRes()
{
	JobDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Job"));
	JobDataTable->AddToRoot();

	FJobRow *Row = JobDataTable->FindRow<FJobRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FJobRow*> AllRow;
		JobDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

const FJobRow& FJobRes::GetJob(int JobId)
{
	FJobRow *Job = JobDataTable->FindRow<FJobRow>(FName(*FString::FromInt(JobId)), FString(""));
	if (nullptr == Job) {
		Job = &DefaultRow;
	}
	
	return *Job;
}
