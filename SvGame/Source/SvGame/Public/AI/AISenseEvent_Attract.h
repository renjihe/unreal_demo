// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "AISense_Attract.h"
#include "Perception/AISenseEvent.h"
#include "AISenseEvent_Attract.generated.h"

UCLASS()
class SVGAME_API UAISenseEvent_Attract : public UAISenseEvent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	FAIAttractEvent Event;

public:
	UAISenseEvent_Attract(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual FAISenseID GetSenseID() const override;
	
	FORCEINLINE FAIAttractEvent GetAttractEvent()
	{
		Event.Compile();
		return Event;
	}
};
