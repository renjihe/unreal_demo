// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AI/Navigation/RecastNavMesh.h"
#include "RecastNavMeshSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API ARecastNavMeshSV : public ARecastNavMesh
{
	GENERATED_BODY()
	
public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
};
