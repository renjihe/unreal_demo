// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AISense_PenetrateSight.h"
#include "AISenseConfig_PenetrateSight.generated.h"

UCLASS(meta = (DisplayName = "AI Penetrate Sight config"))
class SVGAME_API UAISenseConfig_PenetrateSight : public UAISenseConfig_Sight
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<UAISense> GetSenseImplementation() const { return UAISense_PenetrateSight::StaticClass(); };
};
