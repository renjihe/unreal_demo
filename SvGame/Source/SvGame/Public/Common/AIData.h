// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIData.generated.h"

UENUM(BlueprintType)
namespace EChooseTarget
{
	enum Type
	{
		DIS_MIN,
		DIS_MAX,
		HP_MIN,
		HP_MAX,
	};
}

UENUM(BlueprintType)
namespace EStateType
{
	enum Type
	{
		None,
		Disable,
		Idle,
		Follow,
		Attack,
		Cure,
		Patrol,
		Rescue,
	};
}

UENUM(BlueprintType)
namespace EActiveType
{
	enum Type
	{
		None,
		SIGHT,
		HEARING,
		DAMAGE,
	};
}

USTRUCT()
struct FActiveTypeFlags
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = SenseType)
	uint32 SIGHT : 1;

	UPROPERTY(EditAnywhere, Category = SenseType)
	uint32 HEARING : 1;

	UPROPERTY(EditAnywhere, Category = SenseType)
	uint32 DAMAGE : 1;

	UPROPERTY(EditAnywhere, Category = SenseType)
	uint32 ATTRACT : 1;

	bool IsSight() { return SIGHT != 0; }
	bool IsHearing() { return HEARING != 0; }
	bool IsDamage() { return DAMAGE != 0; }
	bool IsAttract() { return ATTRACT != 0; }
};

UENUM(BlueprintType)
namespace ESpeedType
{
	enum Type
	{
		None,
		Patrol,
		Walk,
		Jog,
		Run,
		Push,
		Count,
	};
}

UENUM(BlueprintType)
namespace ESightType
{
	enum Type
	{
		None,
		Normal,
		Penetrate,
	};
}

USTRUCT(BlueprintType)
struct FAIExtra
{
	GENERATED_USTRUCT_BODY()

	FAIExtra()
	{
		SightAngle = 180;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EStateType::Type> OriginState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "180"))
	int SightAngle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ESightType::Type> SightType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int WarnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int FollowRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int PatrolRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bBuildSubPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int SubRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FActiveTypeFlags ActiveType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UNavigationQueryFilter> NavQueryFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TEnumAsByte<ESpeedType::Type>> SpeedTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> TargetPointNames;
};
