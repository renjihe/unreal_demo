// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "DismemberActor.generated.h"

UCLASS()
class SVGAME_API ADismemberActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ADismemberActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent *Mesh;

	class USkeletalMeshComponent* GetMesh() const;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};

FORCEINLINE class USkeletalMeshComponent* ADismemberActor::GetMesh() const {
	return Mesh;
}