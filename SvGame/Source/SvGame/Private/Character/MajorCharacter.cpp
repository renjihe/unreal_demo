// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "Character/MotionControllerProp.h"
#include "MajorCharacter.h"
#include "PlayerStateSV.h"
#include "HVInterface.h"
#include "PlayerControllerSV.h"
#include "GameInstanceSV.h"
#include <UnrealNetwork.h>

AMajorCharacter::AMajorCharacter(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	//MotionClass = TSubclassOf<AMotionController_Base>(AMotionController_Base::StaticClass);
	SimulationHand = CreateDefaultSubobject<USceneComponent>(TEXT("SimulationHand"));
	SimulationHand->SetupAttachment(FirstPersonCameraComponent);
	SimulationHand->RelativeLocation = FVector(81, 0, 0);
	SimulationHand->RelativeRotation = FRotator(60, 0, 0);

	//DisableImpulse = true;
	DisableDismember = true;
	CharRole = ECharacterRole::MAJOR;
	TeamID = 0;
}

void AMajorCharacter::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;

	/*if (MotionController)
	{
		AMotionControllerProp* motionControllerProp = Cast<AMotionControllerProp>(MotionController);
		if (motionControllerProp) {
			motionControllerProp->CreateDefaultProp();
		}
	}*/
}

void AMajorCharacter::BeginDestroy()
{
	Super::BeginDestroy();

	if (MotionController && GetWorld()) {
		GetWorld()->DestroyActor(MotionController);
	}
}

void AMajorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	

	if (Role == ROLE_AutonomousProxy || (Role == ROLE_Authority && IsPlayerControlled() && IsLocallyControlled()))
	{
		if (MotionController && MotionControllerComp->IsVisible()) {
			MotionController->ServerSetControllerTrans(MotionControllerComp->GetRelativeTransform());
		}

		if (CharacterName.IsEmpty() && UHvInterface::hasInit()) {
			ServerSetCharacterName(UHvInterface::getUserName());
		}
	}
}

UMeshComponent* AMajorCharacter::GetSpecMesh(ESpecMesh::Type type)
{
	switch (type)
	{
	case ESpecMesh::Self:
		return GetMesh();
	case ESpecMesh::GripActor:
	{
		if (MotionController)
		{
			AActor* grab = MotionController->GetGrabActor();
			if (grab)
			{
				return Cast<UMeshComponent>(grab->FindComponentByClass(UMeshComponent::StaticClass()));
			}
		}
	}
		return nullptr;
	default:
		return nullptr;
	}

	return nullptr;
}

AActor* AMajorCharacter::GetSpecActor(ESpecMesh::Type type)
{
	if (ESpecMesh::Self == type) {
		return this;
	}
	else if (ESpecMesh::GripActor == type)
	{
		if (MotionController) {
			return MotionController->GetGrabActor();
		}
	}

	return nullptr;
}

void AMajorCharacter::AimTarget(AActor* target, FName &boneName)
{
	AMotionControllerProp* prop = Cast<AMotionControllerProp>(MotionController);
	if (prop)
	{
		prop->MoveAimRotation(GetTargetLocationAndBone(target, boneName));
	}
}


bool AMajorCharacter::Trigger(bool bPressed)
{
	bool ret = false;
	AbilityTouchOff(EAbilityBar::Trigger, bPressed);
	if (MotionController)
	{
		if (bPressed) {
			ret = MotionController->TriggerPressed();
		}
		else {
			ret = MotionController->TriggerReleased();
		}
	}
	return ret;
}

bool AMajorCharacter::Grip1(bool bPressed)
{
	bool ret = false;
	if (MotionController)
	{
		if (bPressed) {
			ret = MotionController->Grip1Pressed();
		}
		else {
			ret = MotionController->Grip1Released();
		}
	}
	return ret;
}

bool AMajorCharacter::Grip2(bool bPressed)
{
	bool ret = false;
	if (MotionController)
	{
		if (bPressed) {
			ret = MotionController->Grip2Pressed();
		}
		else {
			ret = MotionController->Grip2Released();
		}
	}
	return ret;
}

bool AMajorCharacter::Thumbstick(bool bPressed)
{
	bool ret = false;
	if (MotionController)
	{
		if (bPressed) {
			ret = MotionController->ThumbstickPressed();
		}
		else {
			ret = MotionController->ThumbstickReleased();
		}
	}
	return ret;
}

void AMajorCharacter::ThumbstickX(float fAsixValue)
{
	if (MotionController) {
		MotionController->ThumbstickX(fAsixValue);
	}
}

void AMajorCharacter::ThumbstickY(float fAsixValue)
{
	if (MotionController) {
		MotionController->ThumbstickY(fAsixValue);
	}
}

void AMajorCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetMesh()->RelativeLocation.Z = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	GetMesh()->bCastDynamicShadow = true;
	GetMesh()->SetCastShadow(true);
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetOwnerNoSee(true);

	UpdateLook();
}

void AMajorCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AMajorCharacter::PossessedBy(AController *newController)
{
	Super::PossessedBy(newController);

	if (Role == ROLE_Authority && !MotionController)
	{
		//InitAbility();
		MotionController = GetWorld()->SpawnActor<AMotionControllerSV>(MotionClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (MotionController)
		{
			MotionController->SetOwner(this);
			MotionController->Instigator = this;
			MotionController->Camera = FirstPersonCameraComponent;	

			/*AMotionControllerProp* prop = Cast<AMotionControllerProp>(MotionController);
			if (prop)
			{
				prop->CreateDefaultProp();
				prop->OnRep_Griptarget();
				
			}*/

			OnRep_MotionController();
		}
	}	/**/
}

bool AMajorCharacter::ServerMotionControllerAttachTo_Validate(USceneComponent *attachTo)
{
	return true;
}

void AMajorCharacter::ServerMotionControllerAttachTo_Implementation(USceneComponent *attachTo)
{
	if (MotionController) {
		MotionController->AttachToComponent(attachTo, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true));
	}
}

void AMajorCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	UpdateLook();
}

void AMajorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CONDITION(AMajorCharacter, MotionController, COND_SkipOwner);
	DOREPLIFETIME(AMajorCharacter, MotionController);

	DOREPLIFETIME(AMajorCharacter, CharacterName);
}

void AMajorCharacter::OnRep_PlayerState()
{
	UE_LOG(LogTemp, Log, TEXT("[AMajorCharacter::OnRep_PlayerState]%s(%d)%x"), *CharacterName, GetUniqueID(), PlayerState);

	Super::OnRep_PlayerState();	
}

void AMajorCharacter::OnRep_MotionController()
{
	UE_LOG(LogTemp, Log, TEXT("[AMajorCharacter::OnRep_MotionController]%s(%d)%x"), *CharacterName, GetUniqueID(), MotionController);

	if (MotionController)
	{
		if (Controller && (Controller->IsLocalPlayerController() || Controller->Role == ROLE_Authority))
		{
			USceneComponent *attachParent = GetGameInstance() && UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected() ? VRCameraBase : SimulationHand;
			ServerMotionControllerAttachTo(attachParent);
		}
		
		MotionController->Camera = FirstPersonCameraComponent;
		MotionController->Instigator = this;

		APlayerControllerSV *myController = Cast<APlayerControllerSV>(GetWorld()->GetFirstPlayerController());
		if (myController) {
			myController->ServerReplicateTrans(MotionController);
		}
		//MotionController->ServerReplicateTrans();	// MotionController在client创建的时候，同步了WorldTransform(此时还没有AttachParent), 导致计算出错误的RelativeTransform; 所以这里再强制同步下；
	}
}

void AMajorCharacter::OnRep_Controller()
{
	UE_LOG(LogTemp, Log, TEXT("[AMajorCharacter::OnRep_Controller]%s(%d)%x"), *CharacterName, GetUniqueID(), Controller);

	Super::OnRep_Controller();
}

bool AMajorCharacter::ServerSetCharacterName_Validate(const FString &newName)
{
	return true;
}

void AMajorCharacter::ServerSetCharacterName_Implementation(const FString &newName)
{
	CharacterName = newName;
}

void AMajorCharacter::OnRep_CharacterName()
{
	UE_LOG(LogTemp, Log, TEXT("[AMajorCharacter::OnRep_CharacterName]%s(%d)"), *CharacterName, GetUniqueID());
}