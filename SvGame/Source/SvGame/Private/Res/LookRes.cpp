// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "LookRes.h"
#include <Runtime/XmlParser/Public/XmlFile.h>


FLookRes& FLookRes::Get()
{
	static FLookRes ins;
	return ins;
}

FLookRes::FLookRes()	
{
	//DefaultLook.AnimClass = NULL;
	//LibAnimClass = UObjectLibrary::CreateLibrary(UAnimBlueprint::StaticClass(), true, GIsEditor);
	//LibAnimClass->AddToRoot();
	//LibMesh = UObjectLibrary::CreateLibrary(USkeletalMesh::StaticClass(), false, GIsEditor);
	//LibMesh->AddToRoot();

	LookDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/Look"));
	LookDataTable->AddToRoot();

	FLookRow *row = LookDataTable->FindRow<FLookRow>(FName("0"), FString(""));
	if (row) {
		DefaultRow = *row;
	}
	else 
	{
		TArray<FLookRow*> allRow;
		LookDataTable->GetAllRows(FString(""), allRow);
		if (allRow.Num()) {
			DefaultRow = *allRow[0];
		}
	}


	AnimListTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/AnimList"));
	if (AnimListTable)
	{
		AnimListTable->AddToRoot();
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "FLookRes LoadObject fail!!!");
	}
	
}

FLookRow& FLookRes::GetLook(int lookId)
{
	FLookRow *look = LookDataTable->FindRow<FLookRow>(FName(*FString::FromInt(lookId)), FString("FLookRes::GetLook"));
	if (nullptr == look) {
		look = &DefaultRow;
	}
	
	return *look;
}

FAnimList* FLookRes::GetAnimList(int animListId) const
{
	return AnimListTable->FindRow<FAnimList>(FName(*FString::FromInt(animListId)), "FLookRes::GetAnimList");	
}

bool FLookRes::CreateLook(int lookId, USkeletalMeshComponent *mesh)
{
	/*
	if (LibMesh->LoadAssetDataFromPath(look.Mesh))
	{
		TArray<USkeletalMesh*> outs;
		LibMesh->GetObjects(outs);
		if (outs.Num()) {
			mesh->SkeletalMesh = outs[0];
		}
	}

	//const ConstructorHelpers::FObjectFinder<USkeletalMesh> meshLoad(look.Mesh);
	//mesh->SkeletalMesh = mesh.Object;

	if (look.AnimClass) {
		mesh->SetAnimInstanceClass(look.AnimClass);
	}
	*/

	FLookRow &row = GetLook(lookId);
	//if (row.Mesh.IsNull()) 
	{
		row.Mesh.LoadSynchronous();
		if (!row.Mesh.IsNull()) 
		{
			mesh->SetSkeletalMesh(row.Mesh.Get());//mesh->SkeletalMesh = row.Mesh.Get();

			if (row.MaterialGroup.Num() > 0)
			{
				mesh->SkeletalMesh->Materials.Empty();

				for (int i = 0; i < row.MaterialGroup.Num(); ++i)
				{
					int num = row.MaterialGroup[i].Materials.Num();
					num = FMath::RandRange(0, num - 1);

					row.MaterialGroup[i].Materials[num].Material.LoadSynchronous();
					mesh->SkeletalMesh->Materials.Add(FSkeletalMaterial(row.MaterialGroup[i].Materials[num].Material.Get(), true, false
																		, row.MaterialGroup[i].Materials[num].SlotName 
																		, row.MaterialGroup[i].Materials[num].SlotName));
				}
			}
		}
	}
	
	//if (row.Anim.IsValid()) 
	{
		row.Anim.LoadSynchronous();
		if (!row.Anim.IsNull()) {
			mesh->SetAnimInstanceClass(row.Anim->GetAnimBlueprintGeneratedClass());
		}
		
	}	
	return true;
}

void FLookRes::SetAnimation(USkeletalMeshComponent *meshComponent, int lookId, int animType, bool randAnim)
{
	FLookRow& look = GetLook(lookId);
	FAnimList *animList = GetAnimList(look.AnimListId);
	if (nullptr == animList) {
		return;
	}
	
	TAssetPtr<UAnimationAsset> *asset = nullptr;
	if (!randAnim) {
		asset = animList->Anims.Find(animType);
	}
	else
	{
		TArray<TAssetPtr<UAnimationAsset>*> rands;
		for (int i = 0;; ++i)
		{
			TAssetPtr<UAnimationAsset> *assetN = animList->Anims.Find(animType + i * AnimationType::Count);
			if (assetN) {
				rands.Add(assetN);
			}
			else {
				break;
			}
		}

		if (rands.Num()) {
			asset = rands[UKismetMathLibrary::RandomInteger(rands.Num())];
		}
	}

	if (asset)
	{
		asset->LoadSynchronous();
		if (!asset->IsNull()) {
			meshComponent->SetAnimation(asset->Get());
		}
	}
}