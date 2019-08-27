// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SpStaticMeshActor.h"
#include "SpGameMode.h"
#include "EditLevelGameMode.generated.h"

USTRUCT()
struct FSprayMesh
{
	GENERATED_BODY()

	UPROPERTY()
	ASpStaticMeshActor *actor;

	UPROPERTY()
	UStaticMeshComponent *comp;

	UPROPERTY()
	UMaterialInstanceDynamic *mat;
};

/**
 * 
 */
UCLASS()
class AEditLevelGameMode : public ASpGameMode
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	UMaterial *DefaultSprayMaterial;

	UPROPERTY()
	UMaterial *WorldPosMaterial;

	UPROPERTY()
	UMaterial *WorldNormalMaterial;

	UMaterial *CurrSprayMeshMaterial;

	UPROPERTY()
	UTextureRenderTarget2D *RtHDR;

	UPROPERTY()
	UTextureRenderTarget2D *RtLDR;

	TArray<FSprayMesh> SprayMeshs;
public:
	virtual void StartPlay() override;

	UFUNCTION(BlueprintCallable)
	void Save();	
};
