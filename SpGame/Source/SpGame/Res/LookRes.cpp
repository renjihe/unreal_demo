// Fill out your copyright notice in the Description page of Project Settings.

#include "LookRes.h"
#include "SpGame.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

FLookRes& FLookRes::Get()
{
	static FLookRes ins;
	return ins;
}

FLookRes::FLookRes()	
{
	LookDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/Look"));
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
			mesh->SetRelativeRotation(FRotator(0, row.FixYaw, 0));

			if (row.MaterialGroup.Num() > 0)
			{
				//mesh->SkeletalMesh->Materials.Empty();

				for (int i = 0; i < row.MaterialGroup.Num(); ++i)
				{
					int num = row.MaterialGroup[i].Materials.Num();
					num = FMath::RandRange(0, num - 1);

					row.MaterialGroup[i].Materials[num].Material.LoadSynchronous();
					//mesh->SkeletalMesh->Materials.Add(FSkeletalMaterial(row.MaterialGroup[i].Materials[num].Material.Get(), true, false
					//													, row.MaterialGroup[i].Materials[num].SlotName 
					//													, row.MaterialGroup[i].Materials[num].SlotName));
					mesh->SetMaterial(i, row.MaterialGroup[i].Materials[num].Material.Get());
				}
			}

			row.Mesh.Get()->UpdateUVChannelData(false);
		}
	}
	
	//if (row.Anim.IsValid()) 
	{
		if (!row.Anim.ToSoftObjectPath().GetAssetPathName().IsNone())
		{
			UAnimBlueprintGeneratedClass* cls = nullptr;

#if UE_EDITOR
			row.Anim.LoadSynchronous();
			cls = row.Anim->GetAnimBlueprintGeneratedClass();
#else
			FSoftObjectPath path = row.Anim.ToSoftObjectPath();
			path.SetPath(path.GetAssetPathName().ToString() + "_C");
			cls = Cast<UAnimBlueprintGeneratedClass>(path.TryLoad());
#endif

			mesh->SetAnimInstanceClass(cls);
		}
	}

	return true;
}
