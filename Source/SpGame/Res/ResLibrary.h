// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ParticleRes.h"
#include "LookRes.h"
#include "TextureRes.h"
#include "SprayMaskRes.h"
#include "SprayColorRes.h"
#include "AvatarRes.h"
#include "GlobalConfigRes.h"
#include "ResLibrary.generated.h"

UCLASS()
class SPGAME_API UResLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	static const FParticleRow &GetParticleByID(const char *ParticleId);

	static const FLookRow &GetLookByID(int lookId);

	static const FTextureRow &GetTextureByID(int textureId);

	static const FSprayMaskRow &GetSprayMaskByID(int maskId);

	static const FSprayColorRow &GetSprayColorByID(int colorId);

	static const FAvatarRow &GetAvatarByID(int avatarId);
};
