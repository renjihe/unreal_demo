// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Perception/AISense.h"
#include "AISense_Attract.generated.h"


class UAISenseConfig_Attract;
class UAISenseEvent;

USTRUCT()
struct SVGAME_API FAIAttractEvent
{
	GENERATED_USTRUCT_BODY()

	typedef class UAISense_Attract FSenseClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	FVector AttractLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense", meta = (UIMin = 0, ClampMin = 0))
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	FName Tag;

	FGenericTeamId TeamIdentifier;

	FAIAttractEvent();
	FAIAttractEvent(AActor* InInstigator, const FVector& InAttractLocation, float InMaxRange = 0.f, FName Tag = NAME_None);

	void Compile();
};

UCLASS()
class SVGAME_API UAISense_Attract : public UAISense
{
	GENERATED_UCLASS_BODY()
	
protected:
	UPROPERTY()
	TArray<FAIAttractEvent> AttractEvents;

	struct FDigestedAttractProperties
	{
		float AttractRange;
		uint8 AffiliationFlags;

		FDigestedAttractProperties(const UAISenseConfig_Attract& SenseConfig);
		FDigestedAttractProperties();
	};
	TMap<FPerceptionListenerID, FDigestedAttractProperties> DigestedProperties;

public:
	void RegisterEvent(const FAIAttractEvent& Event);
	virtual void RegisterWrappedEvent(UAISenseEvent& PerceptionEvent) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Perception", meta = (WorldContext = "WorldContext"))
	static void ReportAttractEvent(UObject* WorldContext, FVector AttractLocation, AActor* Instigator = nullptr, float MaxRange = 0.f, FName Tag = NAME_None);

protected:
	virtual float Update() override;

	void OnNewListenerImpl(const FPerceptionListener& NewListener);
	void OnListenerUpdateImpl(const FPerceptionListener& UpdatedListener);
	void OnListenerRemovedImpl(const FPerceptionListener& UpdatedListener);
};
