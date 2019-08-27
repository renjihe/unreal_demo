// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Common/GameData.h"
#include "GraspedProp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangeData, AGraspedProp*, prop);

USTRUCT(BlueprintType)
struct FMass
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaggyLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UpVectorStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionStiffness;

	FMass()
	{
		Value = 0.0f;
		LaggyLength = 50.0f;
		ForwardStiffness = 120.0f;
		UpVectorStiffness = 50.0f;
		PositionStiffness = 200.0f;
	}
};

UCLASS()
class SVGAME_API AGraspedProp : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Prop)
	class USkeletalMeshComponent* PropBody;
	
public:	
	// Sets default values for this actor's properties
	AGraspedProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION(BlueprintCallable, Category = "Props")
	bool CanBePackup();

	bool CanThrownAway();

	//UFUNCTION(BlueprintCallable, Category = "Props")
	virtual void PickedUpStart(APawn* Pawn);

	UFUNCTION(BlueprintNativeEvent, Category = "Prop")
	void PickedUpEnd();

	//UFUNCTION(BlueprintCallable, Category = "Props")
	virtual void ThrownAway(bool PutInPackage = false);

	//UFUNCTION(BlueprintCallable, Category = "Props")
	void SelectedByGrabActor(bool bSelect);

	//UFUNCTION(BlueprintCallable, Category = "Props")
	EPropsType::Type GetPropType();

	//UFUNCTION(BlueprintCallable, Category = "Props")
	FMass GetPropMass();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prop")
	FTransform GetGripTransform();

	UPrimitiveComponent* GetGripComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prop")
	FVector GetTraceWorldLocation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prop")
	FRotator GetTraceWorldRotation();

	virtual void SetCurData(int data1, int data2 = 0.0f);

public:
	void SetThrownAway(bool newThrownAway);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	FMass Mass;

	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	TEnumAsByte<EPropsType::Type> PropType;

	UPROPERTY(EditDefaultsOnly, Category = "Prop")
	int CustomDepthStencilValue;

	UPROPERTY(Replicated)
	bool IsCanThrown;

	UPROPERTY(Replicated)
	bool IsAlreadyPicked;
	
public:
	FChangeData ChangeData;
};
