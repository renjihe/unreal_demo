#pragma once
#include "GameData.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterSVMovementComponent.generated.h"

UCLASS()
class SVGAME_API UCharacterSVMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:
	UCharacterSVMovementComponent();

	UPROPERTY(BlueprintReadOnly, Category = MovementMode)
	TEnumAsByte<enum ECustomMovementMode::Type> CustomMovementModeSV;

	UPROPERTY(BlueprintReadOnly)
	bool bClimbMode;

	UPROPERTY(BlueprintReadOnly)
	bool bClimbing;

	UPROPERTY(BlueprintReadOnly)
	bool bClimbTop;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual void PhysicsVolumeChanged(class APhysicsVolume* NewVolume) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	

	void PhyCusClimb(float deltaTime, int32 Iterations);
	UFUNCTION(BlueprintCallable, Category = "Components")
	void ResetMovementMode();
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPhysicsSettings* phySeting;
	bool bPrepareClimb;
	bool bFirstHit;
	bool bFixAngle;
	FVector preVelocity;
};

