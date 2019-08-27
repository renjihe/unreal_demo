// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Common/GameData.h"
#include "Abilities/GameplayAbilityTypes.h"
//#include "Effect/HitImpactEffect.h"
#include "AbilityInfoRes.generated.h"

class UGameplayAbilitySV;
class AHitImpactEffect;

#define INVALID_ABILITY -1

USTRUCT()
struct FAbilityState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ActiveTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FGameplayAbilitySpecHandle GameplayAbilitySpec;					//技能类的句柄

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString Describe;

	FAbilityState()
	{
		AbilityID = 0;
		CD = 0.0f;
		IsRunning = false;

	}
};

USTRUCT(BlueprintType)
struct FFireTrace
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FHitResult> Hits;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector TrailStartPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector TrailEndPoint;
};


/*UENUM(BlueprintType)
namespace EAbilityBar
{
	enum Type
	{
		Trigger,			//
		Push,		//
		Obligate1,
		Obligate2,
		Obligate3,
		Obligate4,

		Count,
	};
}*/

UENUM(BlueprintType)
namespace EAttackType
{
	enum Type
	{
		Projectile,		//子弹攻击
		Melee,			//挥砍攻击
		Flame,			//火焰攻击
		Bomb,			//炸弹攻击
		UnDefined,		//未定义的攻击
		Count,
	};
}

USTRUCT(BlueprintType)
struct FAbilityTypeFlags : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 ATTACK : 1;
	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 AOE : 1;
	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 COMA : 1;
	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 REDUCE_SPEED : 1;
	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 DEF : 1;
	UPROPERTY(EditAnywhere, Category = Flags)
	uint32 CURE : 1;

	bool IsAttack() { return ATTACK != 0; }
	bool IsAoe() { return AOE != 0; }
	bool IsComa() { return COMA != 0; }
	bool IsReduceSpeed() { return REDUCE_SPEED != 0; }
	bool IsDef() { return DEF != 0; }
	bool IsCure() { return CURE != 0; }
};

#define IS_ATTACK_TYPE(Flags) ((Flags & EAbilityType::ATTACK) != 0)
#define IS_CURE_TYPE(Flags) ((Flags & EAbilityType::CURE) != 0)

/**
* FAbilityInfoRow
*/
USTRUCT(BlueprintType)
struct FAbilityInfoRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	//技能类名
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbilitySV> GameAbility;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAbilityTypeFlags AbilityFlags;

	//技能值（攻击力、加血量、减速值等）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Value;

	//技能范围
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	//施法前摇
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AbilityAnimation;

	//持续时间（0为瞬时产生）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration;								

	//技能CD
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AHitImpactEffect> ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 Loop : 1;

	//技能整型参数
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int> ParamInts;

	//技能字符串参数
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> ParamStrings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;

	FAbilityInfoRow()
	{
		Loop = true;
	}
};

class SVGAME_API AbilityInfoRes
{
	
	UDataTable* AbilityDataTable;

	AbilityInfoRes();
public:		
	static AbilityInfoRes& Get();

	bool IsExist(int AbilityID);

	FAbilityInfoRow& GetAbilityinfo(int AbilityID);

	static FAbilityInfoRow	DefaultRow;
};
