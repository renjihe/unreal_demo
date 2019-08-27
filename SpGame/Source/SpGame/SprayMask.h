#pragma once

#include "Object.h"
#include "SprayMask.generated.h"

UCLASS()
class SPGAME_API USprayMask : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	uint32 Width;

	UPROPERTY()
	uint32 Height;

	UPROPERTY()
	TArray<FColor> Pixels;

	int GetMask(float u, float v) const;
};