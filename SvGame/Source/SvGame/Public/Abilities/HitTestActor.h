// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameData.h"
#include "Components/SphereComponent.h"
#include "HitTestActor.generated.h"

/*USTRUCT()
struct FAttachSphereInfoInstance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent *SphereCollosion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ParentSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Add;
};
*/
namespace EHitTestShape
{
	enum Type {
		SPHERE,
		BOX,
	};
}

class USceneComponent;
class UPrimitiveComponent;


/**
 * 
 */
UCLASS()
class SVGAME_API AHitTestActor : public AActor
{
	GENERATED_BODY()
	friend class UGameplayAbilitySV;

	EHitTestShape::Type ShapeType;
	class UGameplayAbilitySV *OwnerAbility;

	float Radius, Duration, PeriodInterval;
	EHitTestMode::Mode TestMode;
	EAbilityTarget::Type TargetType;
	int TargetCount;

	bool DrawDebug;
	//float Age, PeriodAge;
public:
	AHitTestActor();

	bool InitSphere(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, float radius);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void HitTest();

	void PostCreateShape();

	//void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult);
	
	FTimerHandle TimerHandleDuration, TimerHandlePeriod;
	void OnTimerLifeEnd();
	void OnTimerPeriod();
};
