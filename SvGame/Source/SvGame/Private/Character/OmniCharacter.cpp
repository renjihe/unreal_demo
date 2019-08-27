// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "OmniCharacter.h"
#include "GameInstanceSV.h"
#include "HVInterface.h"
#include "OmniControllerComponent.h"
#include "CameraComponentSV.h"
#include "Kismet/KismetMathLibrary.h"
//#include "Kismet/KismetSystemLibrary.h"
#include "Framework/Commands/InputChord.h"


// Sets default values
AOmniCharacter::AOmniCharacter(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{	
	VRCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("VRCameraBase"));
	VRCameraBase->SetupAttachment(GetCapsuleComponent());
	VRCameraBase->RelativeLocation = FVector(0, 0, -88); // Position the camera

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponentSV>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(VRCameraBase);
	FirstPersonCameraComponent->bLockToHmd = true;
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 176); // Position the camera
																	   
	MotionControllerComp = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	MotionControllerComp->SetupAttachment(VRCameraBase);
	MotionControllerComp->Hand = EControllerHand::Right;
	MotionControllerComp->PlayerIndex = INDEX_NONE;

	SteamVRChaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>(TEXT("SteamVRChaperone"));

	OmniController = CreateDefaultSubobject<UOmniControllerComponent>(TEXT("OmniController"));
	OmniController->InitOmniDone.AddDynamic(this, &AOmniCharacter::OnInitOmniDone);

	MaxSpeed = 1;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;

	CondVelocity = 30;
}

// Called when the game starts or when spawned
void AOmniCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Controller && Controller->IsLocalPlayerController())
	{
		MotionControllerComp->PlayerIndex = 0;
	}
}

// Called every frame
void AOmniCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	APlayerController* pc = Cast<APlayerController>(Controller);
	if (pc && (Role == ROLE_AutonomousProxy || (Role == ROLE_Authority && IsLocallyControlled())))
	{
		bool vr = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
		FirstPersonCameraComponent->bUsePawnControlRotation = !vr;

		if (!FMath::IsNearlyZero(TargetSpeedX - CurrentSpeedX)) {
			CurrentSpeedX = UKismetMathLibrary::FInterpTo(CurrentSpeedX, TargetSpeedX, DeltaTime, 8);
		}

		if (!FMath::IsNearlyZero(TargetSpeedY - CurrentSpeedY)) {
			CurrentSpeedY = UKismetMathLibrary::FInterpTo(CurrentSpeedY, TargetSpeedY, DeltaTime, 8);
		}

		if (!FMath::IsNearlyZero(CurrentSpeedX) || !FMath::IsNearlyZero(CurrentSpeedY))	{
			AddMovementInput(GetActorForwardVector() * CurrentSpeedX + GetActorRightVector() * CurrentSpeedY);
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentSpeedX: 0.2f% ---- CurrentSpeedY:0.2f%"), CurrentSpeedX, CurrentSpeedY));
		}		
		
		RecordControllerLocation(DeltaTime);
	}	

	if (Role == ROLE_AutonomousProxy) {
		ServerSetVRCameraTransform(FirstPersonCameraComponent->RelativeLocation, FirstPersonCameraComponent->RelativeRotation);
	}
}

// Called to bind functionality to input
void AOmniCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxisKey<AOmniCharacter>(EKeys::MouseX, this, &AOmniCharacter::MouseX);
	PlayerInputComponent->BindAxisKey<AOmniCharacter>(EKeys::MouseY, this, &AOmniCharacter::MouseY);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOmniCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOmniCharacter::MoveRight);
	
/*	FInputKeyBinding &W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(FInputGesture(EKeys::W), IE_Pressed, this, &AOmniCharacter::WPressed);
	FInputKeyBinding &W_Released = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::W, IE_Released, this, &AOmniCharacter::WReleased);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::S, IE_Pressed, this, &AOmniCharacter::SPressed);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::S, IE_Released, this, &AOmniCharacter::SReleased);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::A, IE_Pressed, this, &AOmniCharacter::APressed);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::A, IE_Released, this, &AOmniCharacter::AReleased);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::D, IE_Pressed, this, &AOmniCharacter::DPressed);
	FInputKeyBinding W_Pressed = PlayerInputComponent->BindKey<AOmniCharacter>(EKeys::D, IE_Released, this, &AOmniCharacter::DReleased);*/
}

void AOmniCharacter::MouseX(float val)
{
	if (OmniController->IsDeveloperMode() && val != 0) {
		AddControllerYawInput(val);
	}
}

void AOmniCharacter::MouseY(float val)
{
	if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled() && val != 0) {
		Super::AddControllerPitchInput(-val);
	}
}

void AOmniCharacter::MoveForward(float Val)
{
	//if (Val != 0)	
	{
		TargetSpeedX = MaxSpeed * Val;
	}
}

void AOmniCharacter::MoveRight(float Val)
{
	//if (Val != 0)	
	{
		TargetSpeedY = MaxSpeed * Val;
	}
}

void AOmniCharacter::RecordControllerLocation(float daltaTime)
{
	FTransform motionTrans = MotionControllerComp->GetComponentTransform();
	FVector newLocation = motionTrans.GetRelativeTransform(GetActorTransform()).GetLocation();
	
	if (LocationNum <= 0) 
	{
		LocationNum = 10;
	}

	if (ControllerLocation.Num() >= LocationNum)
	{
		ControllerLocation.RemoveAt(0);
		TickCount.Remove(0);
	}

	ControllerLocation.Add(newLocation);
	TickCount.Add(daltaTime);
	
	if (ControllerLocation.Num() >= LocationNum)
	{
		float ZOffiset = ControllerLocation[ControllerLocation.Num() - 1].Z - ControllerLocation[0].Z;
		float XOffiset = ControllerLocation[ControllerLocation.Num() - 1].X - ControllerLocation[0].X;
		float timedel = 0;
		
		for (int i = 0; i < ControllerLocation.Num(); ++i)
		{
			timedel += TickCount[i];
		}
		
		if (CondVelocity <= (ZOffiset / timedel) && !AbilityBarState[EAbilityBar::Reload])
		{
			//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, FString::SanitizeFloat(ZOffiset / timedel));
			AbilityTouchOff(EAbilityBar::Reload, true);
		}
		else
		{
			if (AbilityBarState[EAbilityBar::Reload]/*IsIndexAbilityUsing(EAbilityBar::Reload)*/)
			{
				AbilityTouchOff(EAbilityBar::Reload, false);
			}
		}


	}

}

bool AOmniCharacter::ServerSetVRCameraTransform_Validate(const FVector &relativeLocation, const FRotator &relativeRotation) {
	return true;
}

void AOmniCharacter::ServerSetVRCameraTransform_Implementation(const FVector &relativeLocation, const FRotator &relativeRotation)
{
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(relativeLocation, relativeRotation);
}

void AOmniCharacter::OnInitOmniDone()
{
	static bool hasInited = false;
	if (!hasInited) {
		hasInited = true;
		UOmniControllerPluginFunctionLibrary::ResetStepCount();
	}
}