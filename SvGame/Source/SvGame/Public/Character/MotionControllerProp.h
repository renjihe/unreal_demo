// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/GameData.h"
#include "Character/MotionControllerSV.h"
#include "Character/CharacterSV.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerProp.generated.h"

class AGraspedProp;
class AAbilityEntity;

USTRUCT()
struct FGraspedPropInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AGraspedProp> PropClass;

	UPROPERTY(BlueprintReadWrite)
	int EntityParamID;

	UPROPERTY(BlueprintReadWrite)
	int Data1;

	UPROPERTY(BlueprintReadWrite)
	int Data2;
};
/**
 * 
 */
UCLASS()
class SVGAME_API AMotionControllerProp : public AMotionControllerSV
{
	GENERATED_BODY()
	
/*
	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class UCapsuleComponent* _TouchCollision;
*/
	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class UPhysicsHandleComponent* GripPhysicsHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	TSubclassOf<AGraspedProp> DefaultProp;

	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	uint32 IsLaggyHand : 1;

public:
	void CreateDefaultProp();

	UFUNCTION()
	void OnRep_CurGriptarget();

	void TryGripTarget(AGraspedProp *gripTarget);

	UFUNCTION(reliable, Server, WithValidation)
	void ServerGripTarget(AGraspedProp *gripTarget);

	UFUNCTION(reliable, Client)
	void ClientGripTarget(bool grib);

	UFUNCTION(reliable, NetMulticast)
	void NetMulticastGripTarget(AGraspedProp *gripTarget);

	void SeamlessTravelCleanUp();

	//UFUNCTION()
	//void Onrep_GripTarget();

	bool GripTarget(AGraspedProp *target);

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurGriptarget)
	class AGraspedProp* CurrGripTarget;

	virtual AGraspedProp* GetGrabActor() override;

	class AGraspedProp* TraceActor;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Prop)
	TArray<FGraspedPropInfo> PropsInfo;
	bool IsPressing;
	float PressingTime;
public:
	// Sets default values for this actor's properties
	AMotionControllerProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual void Tick(float DeltaTime) override;
/*	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Collision")
	void TouchCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	UFUNCTION(BlueprintNativeEvent, Category = "Collision")
	void TouchCollicionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
*/
	//Pickup Trace
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float GrabFraceSphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	float PullDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	FName TraceSocket;

	void TraceForPickups();

	virtual bool Grip1Pressed() override;
	virtual bool Grip1Released() override;
	virtual bool TriggerPressed() override;
	virtual bool TriggerReleased() override;

	//拾取道具
	void TryPickActor(AGraspedProp* Target);
	//计算被抓物体的位置
	//UFUNCTION(reliable, Server, WithValidation)
	void CalcGripTargetTransform(float fDelta);

	UFUNCTION(BlueprintCallable, Category = "Prop")
	void ChangeProp(EPropsType::Type type);

	UFUNCTION(BlueprintCallable, reliable, Server, WithValidation, Category = "Prop")
	void SeverChangeProp(EPropsType::Type type);

	UFUNCTION()
	void ChangeData(AGraspedProp* prop);

	//grip & Laggy Mass
public:
	 
	FVectorSpringState LaggySpringState;
	FVectorSpringState UpVectorSpringState;
	FVectorSpringState MotionSpringState;

	UPROPERTY(Replicated)
	FVector LaggyMassPosition;	

	UPROPERTY(Replicated)
	FVector UpVectorSpring;	

	UPROPERTY(Replicated)
	FVector MotionPosition;

	UPROPERTY(EditDefaultsOnly, Category = "Prop")
		UCurveFloat* GripCurve;

	UPROPERTY(Replicated)
	bool IsGripping;

	UPROPERTY(Replicated)
	float GripTime;

	UPROPERTY(Replicated)
	bool FollowMotion;

	UPROPERTY(Replicated)
	FTransform GrabbedActorStartingInterpTransform;

	FTransform CalculateNewTransform(bool disableLaggyHands);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	UCurveFloat* AimCurve;

	FRotator GoldAimRotatior;
	bool IsMoveAimRotator;
	float MoveAimTime;

	UFUNCTION(BlueprintCallable, Category = "Prop")
	void MoveAimRotation(FVector targetLocation);
};
