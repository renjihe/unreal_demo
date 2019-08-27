// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GraspedProp.h"


// Sets default values
AGraspedProp::AGraspedProp()
{

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PropBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViveMesh"));
	PropBody->SetupAttachment(RootComponent);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	IsCanThrown = true;
	IsAlreadyPicked = false;

	bReplicates = true;
	bReplicateMovement = true;
}

// Called when the game starts or when spawned
void AGraspedProp::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGraspedProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGraspedProp, IsCanThrown);
	DOREPLIFETIME(AGraspedProp, IsAlreadyPicked);
}

// Called every frame
void AGraspedProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AGraspedProp::CanBePackup()
{
	return !IsAlreadyPicked;
}

bool AGraspedProp::CanThrownAway()
{
	return EPropsType::PrimaryWeapon != PropType;
}

void AGraspedProp::PickedUpStart(APawn* Pawn)
{
	IsAlreadyPicked = true;
	Instigator = Pawn;
	if (Pawn) {
		SetOwner(Pawn->Controller);
	}

	//PickedUpEnd();
}

void AGraspedProp::PickedUpEnd_Implementation()
{
	//PropBody->SetHiddenInGame(true, true);
}

void AGraspedProp::ThrownAway(bool PutInPackage)
{
	IsAlreadyPicked = false;
	Instigator = nullptr;
	SetOwner(nullptr);
}

void AGraspedProp::SelectedByGrabActor(bool bSelect)
{
	PropBody->SetRenderCustomDepth(bSelect);
	if (bSelect)
	{
		CustomDepthStencilValue = FMath::Clamp(CustomDepthStencilValue, 0, 255);
		PropBody->SetCustomDepthStencilValue(CustomDepthStencilValue);
	}

}

EPropsType::Type AGraspedProp::GetPropType()
{
	return PropType;
}

FMass AGraspedProp::GetPropMass()
{
	return Mass;
}

FTransform AGraspedProp::GetGripTransform_Implementation()
{
	return FTransform::Identity;
}

UPrimitiveComponent* AGraspedProp::GetGripComponent()
{
	return PropBody;
}

FVector AGraspedProp::GetTraceWorldLocation_Implementation()
{
	return FVector::ZeroVector;
}

FRotator AGraspedProp::GetTraceWorldRotation_Implementation()
{
	return FRotator::ZeroRotator;
}

void AGraspedProp::SetThrownAway(bool newThrownAway)
{
	IsCanThrown = newThrownAway;
}

void AGraspedProp::SetCurData(int data1, int data2)
{

}