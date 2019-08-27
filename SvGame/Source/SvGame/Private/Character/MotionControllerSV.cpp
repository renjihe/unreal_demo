// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "MotionControllerSV.h"
#include "MotionControllerComponent.h"

// Sets default values
AMotionControllerSV::AMotionControllerSV()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetIsReplicated(true);	//replicate AttachParent, AMotionControllerSV::bReplicateMovement;

	ViveMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViveMesh"));
	ViveMesh->SetupAttachment(RootComponent);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
}

// Called when the game starts or when spawned
void AMotionControllerSV::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMotionControllerSV::SetOwner(AActor *newOwner)
{
	Super::SetOwner(newOwner);

	//bool replicateMovement = false;
	//if (GetOwner())
	//{
	//	AController *controller = Cast<AController>(GetOwner());
	//	if (controller && (controller->IsLocalPlayerController() || controller->Role == ROLE_Authority)) {
	//		replicateMovement = true;
	//	}

	//	if (replicateMovement != bReplicateMovement) 
	//	{
	//		SetReplicateMovement(replicateMovement);
	//	}
	//}
}

void AMotionControllerSV::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CHANGE_CONDITION(AMotionControllerSV, ReplicatedMovement, COND_SkipOwner);
	DOREPLIFETIME(AMotionControllerSV, RelativeTransfrom);
	
}

// Called every frame
void AMotionControllerSV::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*AActor *owner = GetOwner();
	if (owner)
	{
		bool isClient = owner->Role == ROLE_AutonomousProxy && owner->GetNetMode() == NM_Client;
		//AController *controller = Cast<AController>(GetOwner());
		//if (controller && controller->IsLocalPlayerController())
		if (isClient)
		{
			ServerSetControllerTrans(MotionController->RelativeLocation, MotionController->RelativeRotation);
		}
	}*/
}

bool AMotionControllerSV::TriggerPressed()
{
	IsTriggerPressed = true;
	return true;
}

bool AMotionControllerSV::TriggerReleased()
{
	IsTriggerPressed = false;
	return true;
}

bool AMotionControllerSV::Grip1Pressed()
{
	IsGrip1Pressed = true;
	return true;
}

bool AMotionControllerSV::Grip1Released()
{
	IsGrip1Pressed = false;
	return true;
}

bool AMotionControllerSV::Grip2Pressed()
{
	IsGrip2Pressed = true;
	return true;
}

bool AMotionControllerSV::Grip2Released()
{
	IsGrip2Pressed = false;
	return true;
}

bool AMotionControllerSV::ThumbstickPressed()
{
	IsThumbstickPressed = true;
	return true;
}

bool AMotionControllerSV::ThumbstickReleased()
{
	IsThumbstickPressed = false;
	return true;
}

void AMotionControllerSV::ThumbstickX(float fAsixValue)
{
	ThumbstickXAsix = fAsixValue;
}

void AMotionControllerSV::ThumbstickY(float fAsixValue)
{
	ThumbstickYAsix = fAsixValue;
}

bool AMotionControllerSV::ServerSetControllerTrans_Validate(const FTransform &newRelative)
{
	return true;
}

void AMotionControllerSV::ServerSetControllerTrans_Implementation(const FTransform &newRelative)
{
	RelativeTransfrom = newRelative;
}

void AMotionControllerSV::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
}


bool AMotionControllerSV::ServerReplicateTrans_Validate() {
	return true;
}

void AMotionControllerSV::ServerReplicateTrans_Implementation()
{
	NetMulticastReplicateTrans(RootComponent->RelativeLocation, RootComponent->RelativeRotation);
}

void AMotionControllerSV::NetMulticastReplicateTrans_Implementation(const FVector &relativeLocation, const FRotator &relativeRotation)
{
	RootComponent->SetRelativeLocationAndRotation(relativeLocation, relativeRotation);
}

AActor* AMotionControllerSV::GetGrabActor()
{
	return nullptr;
}