// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SpStaticMeshActor.generated.h"

struct FSprayMeshActor;
/**
 * 
 */
UCLASS()
class SPGAME_API ASpStaticMeshActor : public AStaticMeshActor
{
	GENERATED_UCLASS_BODY()
	
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FString UniqueName;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSpray;

	FSprayMeshActor *SprayContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSubdMesh;
};
