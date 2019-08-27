// Fill out your copyright notice in the Description page of Project Settings.

#include "TextureRes.h"
#include "SpGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"

FTextureRes& FTextureRes::Get()
{
	static FTextureRes ins;
	return ins;
}

FTextureRes::FTextureRes()	
{
	TextureDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/Texture"));
	TextureDataTable->AddToRoot();

	FTextureRow *row = TextureDataTable->FindRow<FTextureRow>(FName("0"), FString(""));
	if (row) {
		DefaultRow = *row;
	}
	else 
	{
		TArray<FTextureRow*> allRow;
		TextureDataTable->GetAllRows(FString(""), allRow);
		if (allRow.Num()) {
			DefaultRow = *allRow[0];
		}
	}
}

FTextureRow& FTextureRes::GetTexture(int textureId)
{
	FTextureRow *texture = TextureDataTable->FindRow<FTextureRow>(FName(*FString::FromInt(textureId)), FString("FTextureRes::GetLook"));
	if (nullptr == texture) {
		texture = &DefaultRow;
	}
	
	return *texture;
}
