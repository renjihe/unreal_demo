// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "AISense_PenetrateSight.generated.h"

class UAISense_PenetrateSight;
class UAISenseConfig_PenetrateSight;

UCLASS(ClassGroup=AI, config=Game)
class SVGAME_API UAISense_PenetrateSight : public UAISense_Sight
{
	GENERATED_UCLASS_BODY()

protected:
	virtual float Update() override;
};
