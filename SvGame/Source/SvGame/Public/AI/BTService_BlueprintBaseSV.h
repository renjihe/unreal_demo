// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Common/AIData.h"
#include "BTBlueprintBaseInterface.h"
#include "Classes/BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_BlueprintBaseSV.generated.h"

class ACharacterSV;

UCLASS(Abstract, Blueprintable)
class SVGAME_API UBTService_BlueprintBaseSV : public UBTService_BlueprintBase, public IBTBlueprintBaseInterface
{
	GENERATED_BODY()
public:
	virtual void SetOwner(AActor* owner) override;
};
