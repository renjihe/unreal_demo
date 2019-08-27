// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/OmniCharacter.h"
#include "Character/MotionControllerSV.h"
#include "MajorCharacter.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API AMajorCharacter : public AOmniCharacter
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CharacterSV")
	class USceneComponent* SimulationHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_MotionController, Category = "CharacterSV")
	class AMotionControllerSV* MotionController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterSV")
	TSubclassOf<AMotionControllerSV> MotionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CharacterName, Category = "CharacterSV")
	FString CharacterName;

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCharacterName(const FString &newName);

	UPROPERTY(EditAnywhere, Category = "CharacterSV")
	int PawnId;

	UFUNCTION(reliable, server, WithValidation)
	void ServerMotionControllerAttachTo(USceneComponent *attachTo);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController *newController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

	UFUNCTION()
	void OnRep_MotionController();

	UFUNCTION()
	void OnRep_CharacterName();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UMeshComponent* GetSpecMesh(ESpecMesh::Type type) override;
	virtual AActor* GetSpecActor(ESpecMesh::Type type) override;

	virtual void AimTarget(AActor* target, FName &boneName) override;

public:
	UFUNCTION(BlueprintCallable, category = "CharacterController")
	bool Trigger(bool bPressed);

	UFUNCTION(BlueprintCallable, category = "CharacterController")
	bool Grip1(bool bPressed);

	UFUNCTION(BlueprintCallable, category = "CharacterController")
	bool Grip2(bool bPressed);

	UFUNCTION(BlueprintCallable, category = "CharacterController")
	bool Thumbstick(bool bPressed);
	
	UFUNCTION(BlueprintCallable, category = "CharacterController")
	virtual void ThumbstickX(float fAsixValue);

	UFUNCTION(BlueprintCallable, category = "CharacterController")
	virtual void ThumbstickY(float fAsixValue);

};
