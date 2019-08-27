// Fill out your copyright notice in the Description page of Project Settings.

#include "ResLibrary.h"
#include "SpGame.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsPublic.h"

UResLibrary::UResLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const FParticleRow &UResLibrary::GetParticleByID(const char *ParicleId)
{
	return ParticleRes::Get().GetParticle(ParicleId);
}

const FLookRow &UResLibrary::GetLookByID(int LookId)
{
	return FLookRes::Get().GetLook(LookId);
}

const FTextureRow &UResLibrary::GetTextureByID(int textureId)
{
	return FTextureRes::Get().GetTexture(textureId);
}

const FSprayMaskRow &UResLibrary::GetSprayMaskByID(int maskId)
{
	return FSprayMaskRes::Get().GetMask(maskId);
}

const FSprayColorRow &UResLibrary::GetSprayColorByID(int colorId)
{
	return FSprayColorRes::Get().GetColor(colorId);
}

const FAvatarRow &UResLibrary::GetAvatarByID(int avatarId)
{
	return FAvatarRes::Get().GetAvatar(avatarId);
}
