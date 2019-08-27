// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "LookRes.generated.h"

#define Invalid_Look -1

UENUM(BlueprintType)
namespace Dismembermemt
{
	enum Type {
		ArmL = 0,
		ArmR,
		Head,
		LegL,
		LegR,
		Count,
	};
}

UENUM (BlueprintType)
namespace AnimationType
{
	enum Type {
		Melee = 0,
		Fire,
		Sweep,
		Claw,
		Kick,
		Stamp,
		Ability0,
		Ability1,
		Ability2,

		Count = 100,
	};
}

USTRUCT(BlueprintType)
struct FDismemberment
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Bone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DismembermentHP;
};

USTRUCT(BlueprintType)
struct FAnimList: public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32, TAssetPtr<UAnimationAsset>> Anims;
};

USTRUCT(BlueprintType)
struct FSkeletalMaterialTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SlotName;
};

USTRUCT(BlueprintType)
struct FMaterialList : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSkeletalMaterialTableRow> Materials;
};

/**
* FLookRow
*/
USTRUCT(BlueprintType)
struct FLookRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	//UPROPERTY(BlueprintReadOnly)
	//FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UAnimBlueprint> Anim;
	//TSubclassOf<class UAnimBlueprintGeneratedClass> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, TEnumAsByte<Dismembermemt::Type>> DismemberBone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDismemberment> Dismemberment;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TEnumAsByte<EMovementMode>, FName> MovementMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AnimListId;
	//TMap<TEnumAsByte<ECustomMovementMode::Type>, FName> CustomMovementMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMaterialList> MaterialGroup;
};

class SVGAME_API FLookRes
{
	//UObjectLibrary *LibAnimClass;
	//UObjectLibrary *LibMesh;

	FLookRow	DefaultRow;

	UDataTable	*LookDataTable;
	UDataTable	*AnimListTable;

	FLookRes();
public:		
	static FLookRes& Get();

	FLookRow& GetLook(int lookId);
	FAnimList* GetAnimList(int animListId) const;

	bool CreateLook(int lookId, USkeletalMeshComponent *mesh);

	void SetAnimation(USkeletalMeshComponent *meshComponent, int lookId, int animType, bool randAnim = true);
};
