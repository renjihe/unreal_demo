// Fill out your copyright notice in the Description page of Project Settings.

#include "ParticleRes.h"
#include "SpGame.h"

ParticleRes& ParticleRes::Get()
{
	static ParticleRes ins;
	return ins;
}

ParticleRes::ParticleRes()
{
	ParticleDataTable = LoadObject<UDataTable>(NULL, TEXT("/Game/Config/DataTable/Particle"));
	ParticleDataTable->AddToRoot();

	FParticleRow *Row = ParticleDataTable->FindRow<FParticleRow>(FName("0"), FString(""));
	if (Row) {
		DefaultRow = *Row;
	}
	else 
	{
		TArray<FParticleRow*> AllRow;
		ParticleDataTable->GetAllRows(FString(""), AllRow);
		if (AllRow.Num()) {
			DefaultRow = *AllRow[0];
		}
	}
}

FParticleRow& ParticleRes::GetParticle(const char *ParticleId)
{
	FParticleRow *Particle = ParticleDataTable->FindRow<FParticleRow>(FName(*FString(ParticleId)), FString(""));
	if (nullptr == Particle) {
		Particle = &DefaultRow;
	}
	
	return *Particle;
}