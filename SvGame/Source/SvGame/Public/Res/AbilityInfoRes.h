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
		FGameplayAbilitySpecHandle GameplayAbilitySpec;					//������ľ��

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
		Projectile,		//�ӵ�����
		Melee,			//�ӿ�����
		Flame,			//���湥��
		Bomb,			//ը������
		UnDefined,		//δ����Ĺ���
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

	//��������
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbilitySV> GameAbility;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAbilityTypeFlags AbilityFlags;

	//����ֵ������������Ѫ��������ֵ�ȣ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Value;

	//���ܷ�Χ
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	//ʩ��ǰҡ
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AbilityAnimation;

	//����ʱ�䣨0Ϊ˲ʱ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration;								

	//����CD
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AHitImpactEffect> ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint32 Loop : 1;

	//�������Ͳ���
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int> ParamInts;

	//�����ַ�������
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
