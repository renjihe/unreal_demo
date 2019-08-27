// Fill out your copyright notice in the Description page of Project Settings.

#include "AvatarRes.h"
#include "SpGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

FAvatarRes& FAvatarRes::Get()
{
	static FAvatarRes ins;
	return ins;
}

FAvatarRes::FAvatarRes()
{
	DataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/Avatar"));
	DataTable->AddToRoot();

	FAvatarRow *row = DataTable->FindRow<FAvatarRow>(FName("0"), FString(""));
	if (row) {
		DefaultRow = *row;
	}
	else 
	{
		TArray<FAvatarRow*> allRow;
		DataTable->GetAllRows(FString(""), allRow);
		if (allRow.Num()) {
			DefaultRow = *allRow[0];
		}
	}
}

FAvatarRow& FAvatarRes::GetAvatar(int avatarId)
{
	FAvatarRow *Avatar = DataTable->FindRow<FAvatarRow>(FName(*FString::FromInt(avatarId)), FString("FAvatarRes::GetAvatar"));
	if (nullptr == Avatar) {
		Avatar = &DefaultRow;
	}
	
	return *Avatar;
}

