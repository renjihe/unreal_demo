// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HitImpactEffect.generated.h"

USTRUCT()
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	UMaterial* DecalMaterial;

	
	UPROPERTY(EditDefaultsOnly, Category = Decal)
	FVector DecalSize;

	
	FDecalData()
		: DecalSize(FVector::ZeroVector)
	{
	}
};

USTRUCT()
struct FDecalDataArray
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	TArray<FDecalData> Decals;

	UPROPERTY(EditDefaultsOnly, Category = Decal)
	float LifeSpan;

	FDecalDataArray()
	{
		Decals.Reset();
		LifeSpan = 10.0f;
	}
};

UCLASS(Abstract, Blueprintable)
class SVGAME_API AHitImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** spawn effect */
	virtual void PostInitializeComponents() override;

public:

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, int> ParticleMap;

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, int> SoundMap;

	UPROPERTY(EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, FDecalDataArray> DecalMap;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

};
