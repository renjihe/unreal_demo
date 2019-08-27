// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "common/battledef.h"
#include "SprayMgr.generated.h"

class ASpStaticMeshActor;

struct FNeighborVertex
{
	uint16 index;
	char wu, wv;
};

struct FVertexNeighborInfo
{
	uint8 count;
	FNeighborVertex neighbor[16];
};

USTRUCT()
struct FSprayMeshActor 
{
	GENERATED_BODY()

	UPROPERTY()
	ASpStaticMeshActor *actor;
	UStaticMeshComponent *comp;

	TArray<FVertexNeighborInfo> vertex_info;
	void buildVertexNeighborInfo();

	UPROPERTY()
	UMaterialInstanceDynamic *material;

	UPROPERTY()
	class UTextureRenderTarget2D *rt;

	UPROPERTY()
	class UTextureRenderTarget2D *rt2;

	UPROPERTY()
	class UTexture2D *worldpos;

	int size;

	int sprayCount, rtDataDirty;
	TArray<FColor> rtData;

	int atkerSprayScore;
	int deferSprayScore;
	int totalSprayScore;

	void *physicActor;
};

UCLASS()
class ASprayMgr : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	USceneCaptureComponent2D *SceneCapture;

	UPROPERTY()
	UMaterial *ExcuteSpayMaterial;

	UPROPERTY()
	UMaterial *Spray2MeshMaterial;
	
	TArray<FSprayMeshActor> SprayMeshActors;
	static void SwapTextureRenderTarget(FSprayMeshActor &sma);
	static FColor ReadSprayRenderTarget(FSprayMeshActor &sma, const FVector2D &uv);
	static void UpdateScore(FSprayMeshActor &sma);
	FSprayMeshActor* FindSprayMeshActor(AActor *actor);

	struct FSpray {
		FVector loc, dir, size;
		int faction;
		UMaterialInstanceDynamic *mat;
	};
	TArray<FSpray> Sprays;

	UPROPERTY()
	UMaterialInstanceDynamic *ExcuteSpayMaterialDynamic;

	TArray<UMaterialInstanceDynamic*> ExecuteSprayMaterialPool;
	UMaterialInstanceDynamic* NewExecuteSprayMaterial();
	void DelExecuteSprayMaterial(UMaterialInstanceDynamic *mat);
	void CleanExecuteSprayMaterial();

	static void ExecuteSpray(UTextureRenderTarget2D *rt, UMaterialInstanceDynamic *mat);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SprayMode;

private:
	static void ExecuteSprayVertex(int caster, FSprayMeshActor &sma, int faction, float(*m)[4], float range);
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime);
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable)
	static void Spray(int caster, FVector loc, FVector normal, FVector dir, int faction, UTexture2D *mask, FVector size = FVector(100, 100, 100), float range = 0.8f);

	UFUNCTION(BlueprintCallable)
	static void SaveSprayTexture();

	UFUNCTION(BlueprintCallable)
	static int GetSprayFaction(FVector rayOrigin, FVector rayDir = FVector(0, 0, -1));

	UFUNCTION(BlueprintCallable)
	static void DoSpray2(FVector loc, FVector normal, FVector dir, int faction, USprayMask *mask, FVector size);
};
