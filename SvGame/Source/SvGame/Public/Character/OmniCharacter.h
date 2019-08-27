// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterSV.h"
#include "OmniCharacter.generated.h"

class UGameInstanceSV;

UCLASS()
class SVGAME_API AOmniCharacter : public ACharacterSV
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UOmniControllerComponent *OmniController;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class USteamVRChaperoneComponent* SteamVRChaperone;	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterSV")
	class USceneComponent* VRCameraBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSV")
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	class UMotionControllerComponent* MotionControllerComp;
	
	//开发者模式下最大速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterSV")
	float MaxSpeed;

	//开发者模式下模拟的速度
	float CurrentSpeedX;
	float CurrentSpeedY;
	float TargetSpeedX;
	float TargetSpeedY;

	UPROPERTY(BlueprintReadWrite, Category = "CharacterSV")
	float CondVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSV")
	int LocationNum;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSV")
	TArray<FVector> ControllerLocation;
	
	TArray<float>  TickCount;
protected:
	class UGameInstanceSV* MyGameInstance;

	UFUNCTION()
	void OnInitOmniDone();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void MouseX(float val);
	void MouseY(float val);

	void MoveForward(float Val);
	void MoveRight(float Val);

	void RecordControllerLocation(float daltaTime);
	
	UFUNCTION(server, unreliable, WithValidation)
	void ServerSetVRCameraTransform(const FVector &relativeLocation, const FRotator &relativeRotation);
};
