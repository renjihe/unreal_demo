// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "ResLibrary.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsPublic.h"

UResLibrary::UResLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FSoundRow UResLibrary::GetSoundByID(int SoundId)
{
	return SoundRes::Get().GetSound(SoundId);
}

FParticleRow UResLibrary::GetParticleByID(int ParicleId)
{
	return ParticleRes::Get().GetParticle(ParicleId);
}

FShakeRow UResLibrary::GetGunRecoilByID(int GunRecoilId)
{
	return ShakeRes::Get().GetShake(GunRecoilId);
}

FAbilityInfoRow UResLibrary::GetAbilityInfoByID(int AbilityId)
{
	return AbilityInfoRes::Get().GetAbilityinfo(AbilityId);
}

FSpreadRow UResLibrary::GetGunSpreadByID(int GunSpreadId)
{
	return SpreadRes::Get().GetSpread(GunSpreadId);
}

FBTRow UResLibrary::GetBTByID(int BTId)
{
	return FBTRes::Get().GetBT(BTId);
}

FJobRow UResLibrary::GetJobByID(int JobId)
{
	return FJobRes::Get().GetJob(JobId);
}

FLookRow UResLibrary::GetLookByID(int LookId)
{
	return FLookRes::Get().GetLook(LookId);
}

FAbilityEntityInfoRow UResLibrary::GetAbilityEntityInfoByID(int AbilityEntityInfoId)
{
	return AbilityEntityInfoRes::Get().GetAbilityEntityInfo(AbilityEntityInfoId);
}

//FHitTestActorRow UResLibrary::GetHitTestActorByID(int HitTestActorId)
//{
//	return HitTestActorRes::Get().GetHitTestActor(HitTestActorId);
//}

UAnimMontage* UResLibrary::LoadAnimMontage(const FString &Path)
{
	return Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, *(Path)));
}

void UResLibrary::GetBodySetUps(UPhysicsAsset* physiceAsset, TArray<FBodySetupItem> &bodySetups)
{
	for (int i = 0; i < physiceAsset->SkeletalBodySetups.Num(); ++i)
	{
		USkeletalBodySetup* bodySetup = physiceAsset->SkeletalBodySetups[i];
		FBodySetupItem item;
		if (bodySetup->AggGeom.GetElementCount(KPT_Sphyl) > 0)
		{
			item.BoneName = bodySetup->BoneName;
			for (int j = 0; j < bodySetup->AggGeom.GetElementCount(KPT_Sphyl); ++j)
			{
				FKCapsulesItem capsulesItem;
				capsulesItem.Center = bodySetup->AggGeom.SphylElems[j].Center;
				capsulesItem.Orientation = bodySetup->AggGeom.SphylElems[j].Orientation;
				capsulesItem.Radius = bodySetup->AggGeom.SphylElems[j].Radius;
				capsulesItem.Length = bodySetup->AggGeom.SphylElems[j].Length;
				item.Capsules.Add(capsulesItem);
			}
			bodySetups.Add(item);
		}
	}
}