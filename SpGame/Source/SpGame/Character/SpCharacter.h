// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/GameData.h"
#include "GameFramework/Character.h"
#include "common/battledef.h"
#include "SpCharacter.generated.h"

enum AvatarPosition;
class UInputComponent;
class FEntityComponent;

UCLASS()
class SPGAME_API ASpCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	float SprayRate;
	float AtkedRate;
	float AtkedTimer;

	int CurrLookId;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	int LookId;

	bool bInit;
protected:
	virtual void InitSpray();
	virtual void InitAttachmentSpray(AActor *actor);
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void PossessedBy(AController *newController) override;
	virtual USceneComponent* GetDefaultAttachComponent() const;

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void MoveForward(float value);
	void MoveRight(float value);
	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);
	void MouseLeftPress();
	void MouseLeftRelease();
	void MouseRightPress();
	void MouseRightRelease();
	void Jump();
	void StopJumping();
	//void Sneak();
	//void StopSneak();

	void KeyQPress();
	void KeyQRelease();

	void KeyEPress();
	void KeyERelease();

	void UpdateLook();
	void ForEachNeedUpdateMat(TFunctionRef<void(UMaterialInstanceDynamic*)> Predicate);
public:
	void UpdateSprayRate();
	void UpdateAtkedRate();
	void MoveForwardImpl(float value, float Yaw = 0.0f);
	void MoveRightImpl(float value, float Yaw = 0.0f);
	void AddControllerPitchInputImpl(float Val);
	void AddControllerYawInputImpl(float Val);
	void MouseLeftPressImpl();
	void MouseLeftReleaseImpl();
	void MouseRightPressImpl();
	void MouseRightReleaseImpl();

	FEntityComponent *GetEntityComponent() {
		return EntityComponent.Get();
	}

	TSharedPtr<FEntityComponent> EntityComponent;
public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Init(FVector Location, FRotator Rotator);

	UFUNCTION(BlueprintCallable)
	bool IsFalling() const;

	virtual FVector GetVelocity() const override;

	UFUNCTION(BlueprintCallable)
	float GetYawInput() const;

	UFUNCTION(BlueprintCallable)
	float GetPitchInput() const;

	UFUNCTION(BlueprintCallable)
	void PrintSocketInfo(int pos, FName fromSocket, FVector relaLocation = FVector::ZeroVector, FRotator relaRotation = FRotator::ZeroRotator);

	int JobId;
	unsigned EntityId;
	uint32 MoveState;
	float MoveSpeed;
	int Faction;

	UFUNCTION(BlueprintCallable)
	void SetLook(int look);

	UFUNCTION()
	virtual void OnParticleSystemFinished(class UParticleSystemComponent* FinishedComponent);

	void AttachActor(int avatarId, AvatarPosition pos);
public:
	AActor* Avatar[AvatarPosition::Count];

private:
	int CurrAvatarId[AvatarPosition::Count];
	void AttachActor(AActor* childActor, AvatarPosition pos);
};
