// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitiesType.generated.h"

/**
 * 
 */

//Attribute
//////////////////////////////////////////////////////////////////////////
UENUM(BlueprintType)
namespace EAttribType
{
	enum Type
	{
		Health,
		MaxHealth,
		Speed,
		AttackPower,
		ArmorProjectile,		//×Óµ¯»¤¼×
		ArmorMelee,			//»Ó¿³»¤¼×
		ArmorFlame,			//»ðÑæ»¤¼×

		Extremis = 32, //±ôËÀ£¬ÁÙÖÕ
		GainSpeed,
		Count,
	};
}

USTRUCT()
struct FAttribs
{
	GENERATED_USTRUCT_BODY()

	int& operator[](int index) {
		return Attribs[index];
	}

	FAttribs& operator += (const FAttribs &rhs);
	FAttribs& operator -= (const FAttribs &rhs);
	FAttribs& operator * (float n);

	int Attribs[EAttribType::Count];

	FAttribs()
	{
		Attribs[EAttribType::Health] = -1;
		Attribs[EAttribType::AttackPower] = 1;
		Attribs[EAttribType::Speed] = 600;
		Attribs[EAttribType::ArmorProjectile] = 100;
		Attribs[EAttribType::ArmorMelee] = 100;
		Attribs[EAttribType::ArmorFlame] = 100;
	}
};

static const char* ATTRIBUTE_NAME[] = {
	"Attrib_Health",
	"Attrib_MaxHealth",
	"Attrib_Speed",
	"Attrib_AttackPower",
	"Attrib_ArmorProjectile",
	"Attrib_ArmorMelee",
	"Attrib_ArmorFlame"
};
