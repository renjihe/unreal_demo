// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameData.generated.h"

//-------------------------Movement mode-----------------------
///////////////////////////////////////////////////////////////////////////
UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climbing,
		MOVE_MAX,
	};
}

//-------------------- Prop -----------------------------------
UENUM(BlueprintType)
namespace EPropsType
{
	enum Type
	{
		PrimaryWeapon = 0,
		SecondaryWeapan,
		Frag,
		Pakage,
		TemporaryMedicine,
		Other,

		Count,
	};
}

#define IsEffectPositionAttached(position) ((position&1) != 0)

UENUM(BlueprintType)
namespace CharacterBodyPart
{
	enum Type
	{
		Head,	//ͷ
		Body,	//����
		ArmL,	//�첲 ��
		ArmR,	//�첲 ��
		LegL,	//�� ��
		LegR,	//�� ��

		Count,
	};
}

UENUM(BlueprintType)
namespace EAbilityBar
{
	enum Type
	{
		MOUSE_LEFT,			
		MOUSE_RIGHT,		
		Count,
	};
}

UENUM(BlueprintType)
namespace EAbilityTarget
{
	enum Type
	{
		Friend = 1,
		Enemy = 2,
		Neutral = 4,
		EnemyNeutral = 6,		//Enemy + Neutral,
		Self = 8,		
		FriendSelf = 9,			//Friend + Self,
		FriendSelfNeutral = 13, //Friend + Self + Neutral,
		All = 0xFF,
	};
}


UENUM(BlueprintType)
namespace EHitTestMode {
	enum Mode {
		Always,
		Period,
		PeriodOnce,	// �����ԣ�����һ��actorֻ����һ��hit�¼�
	};
}

UENUM(BlueprintType)
namespace ECharacterRole
{
	enum Type
	{
		MOB,
		MAJOR,
		MINION,
		MINION_ATTRACT, //����
	};
}
