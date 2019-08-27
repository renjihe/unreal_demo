// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "ParticleRes.generated.h"
/**
* FParticleRow
*/
USTRUCT(BlueprintType)
struct FParticleRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UParticleSystem* Particle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeDilation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Scale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator Rotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Describe;

	FParticleRow()
	{
		TimeDilation = 1.0f;
		Scale = FVector(1.0f, 1.0f, 1.0f);
	}
};

class SVGAME_API ParticleRes
{
	FParticleRow	DefaultRow;
	UDataTable*		ParticleDataTable;

	ParticleRes();
public:		
	static ParticleRes& Get();

	FParticleRow& GetParticle(int ParticleId);
};
