// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "SteamVRChaperoneComponent.h"
#include "MotionControllerSV.generated.h"

UCLASS()
class SVGAME_API AMotionControllerSV : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "false"), Category = Components)
	class USkeletalMeshComponent* ViveMesh;

	UPROPERTY(Replicated)
	FTransform RelativeTransfrom;
	
public:	
	// Sets default values for this actor's properties
	AMotionControllerSV();

	virtual void SetOwner(AActor *newOwner) override;
	//virtual void PreReplication( IRepChangedPropertyTracker & ChangedPropertyTracker )
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_ReplicatedMovement() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UCameraComponent* Camera;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
	virtual bool TriggerPressed();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
	virtual bool TriggerReleased();
	
	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
	virtual bool Grip1Pressed();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
	virtual bool Grip1Released();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
	virtual bool Grip2Pressed();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
		virtual bool Grip2Released();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
		virtual bool ThumbstickPressed();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
		virtual bool ThumbstickReleased();

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
		virtual void ThumbstickX(float fAsixValue);

	UFUNCTION(BlueprintCallable, category = "MotionControllerBase")
		virtual void ThumbstickY(float fAsixValue);

	UFUNCTION(server, unreliable, WithValidation)
	void ServerSetControllerTrans(const FTransform &newRelative);

	UFUNCTION(reliable, server, WithValidation)
	void ServerReplicateTrans();	// force to synchronize relative transform;

	UFUNCTION(reliable, NetMulticast)
	void NetMulticastReplicateTrans(const FVector &relativeLocation, const FRotator &relativeRotation);

	virtual AActor* GetGrabActor();

	void SetRelativeTransfrom(FTransform newRelative);
protected:
	bool IsTriggerPressed;
	bool IsGrip1Pressed;
	bool IsGrip2Pressed;
	bool IsThumbstickPressed;
	float ThumbstickXAsix;
	float ThumbstickYAsix;
};
