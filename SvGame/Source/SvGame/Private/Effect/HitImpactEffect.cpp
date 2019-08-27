// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "HitImpactEffect.h"
#include "Kismet/KismetMathLibrary.h"

#include "Res/ParticleRes.h"
#include "Res/SoundRes.h"
#include "GameModeSV.h"
#include "GameInstanceSV.h"

AHitImpactEffect::AHitImpactEffect(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAutoDestroyWhenFinished = true;
}

// Called when the game starts or when spawned
void AHitImpactEffect::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHitImpactEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHitImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);
	/*if (HitSurfaceType == SurfaceType_Default){
		return;
	}*/

	//播放特效
	int* particleID = ParticleMap.Find(HitSurfaceType);
	if (particleID)
	{
		FParticleRow impactFX = ParticleRes::Get().GetParticle(*particleID);
		if (impactFX.Particle)
		{
			FVector direction = (FVector(0, 0, 0) - SurfaceHit.ImpactNormal).GetSafeNormal();
			direction = FMath::VRandCone(direction, 0.25);
			direction *= -1;
			FRotator RotatorOffset = UKismetMathLibrary::ComposeRotators(impactFX.Rotator, FRotationMatrix::MakeFromZ(direction).Rotator());
			UParticleSystemComponent* particleComponent = UGameplayStatics::SpawnEmitterAtLocation(this, impactFX.Particle, GetActorLocation(), RotatorOffset);
			particleComponent->CustomTimeDilation = impactFX.TimeDilation;
			particleComponent->SetWorldScale3D(impactFX.Scale);


			//ParticleComponent->SetWorldRotation(FQuat(RotatorOffset));
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Impact Particle is None!"), true, true, FLinearColor(1, 0, 0));
		}
	}
	

	//播放声音
	int* soundID = ParticleMap.Find(HitSurfaceType);
	if (soundID)
	{
		FSoundRow impactSound = SoundRes::Get().GetSound(*soundID);
		if (impactSound.Sound)
		{
			UAudioComponent* ac = UGameplayStatics::SpawnSoundAtLocation(this, impactSound.Sound, GetActorLocation(), GetActorRotation(), impactSound.Volume, impactSound.Pitch);
			//UGameplayStatics::PlaySoundAtLocation(this, impactSound.Sound, GetActorLocation(), impactSound.Volume, impactSound.Pitch);

			if (ac)
			{
				ac->bOverrideAttenuation = true;
				ac->AttenuationOverrides.AttenuationShapeExtents.X = impactSound.Radius;
				ac->AttenuationOverrides.FalloffDistance = impactSound.FalloffDistance;
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Impact Sound is None!"), true, true, FLinearColor(1, 0, 0));
		}
	}
	
	FDecalDataArray* decalArray = DecalMap.Find(HitSurfaceType);
	if (decalArray)
	{
		if (decalArray->Decals.Num() > 0)
		{
			if (SurfaceHit.Component.Get())
			{
				//随机贴花
				int index = FMath::Rand() % decalArray->Decals.Num();
				//随机贴花角度和尺寸
				FRotator randomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
				randomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);
				float randomSize = FMath::RandRange(0.75f, 1.9f);

				//如果有贴花材质
				if (decalArray->Decals[index].DecalMaterial)
				{

					UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAttached(decalArray->Decals[index].DecalMaterial, decalArray->Decals[index].DecalSize * randomSize,
						SurfaceHit.Component.Get(), SurfaceHit.BoneName,
						SurfaceHit.ImpactPoint, randomDecalRotation, EAttachLocation::KeepWorldPosition, decalArray->LifeSpan);

					//AGameModeSV* gameMode = Cast<AGameModeSV>(UGameplayStatics::GetGameMode(this));
					UGameInstanceSV* gameInstance = Cast<UGameInstanceSV>(UGameplayStatics::GetGameInstance(this));
					if (gameInstance)
					{
						FDecalItem decal;
						decal.Comp = DecalComponent;
						decal.AutoDestroyTime = GetWorld()->GetTimeSeconds() + decalArray->LifeSpan;
						gameInstance->AddDecalItem(decal);
					}
				}
			}

		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Impact Decal is None!"), true, true, FLinearColor(1, 0, 0));
		}
	}
	
}

