#include "SvGame.h"
#include "LookRes.h"
#include "CharacterSV.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "CharacterSVMovementComponent.h"


UCharacterSVMovementComponent::UCharacterSVMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	phySeting = UPhysicsSettings::Get();
	//-------------------------------------------------------------
	bClimbMode = false;
	bClimbing = false;
	bClimbTop = false;
	bFixAngle = false;
	bFirstHit = true;
	preVelocity = FVector::ZeroVector;
}

void UCharacterSVMovementComponent::BeginPlay()
{
	CharacterOwner->GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &UCharacterSVMovementComponent::OnComponentBeginOverlap);
	CharacterOwner->GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &UCharacterSVMovementComponent::OnComponentEndOverlap);
	CharacterOwner->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UCharacterSVMovementComponent::OnComponentHit);

}

void UCharacterSVMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterSVMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);
	switch (CustomMovementMode)
	{
	case ECustomMovementMode::MOVE_Climbing:
		PhyCusClimb(deltaTime, Iterations);
		break;
	default:
		break;
	}
}

void UCharacterSVMovementComponent::PhyCusClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		/*if (Acceleration.IsZero())
		{
			Velocity = FVector::ZeroVector;
		}
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());*/
		Velocity = FVector::ZeroVector;
		Velocity.Z = 100 + deltaTime;
	}

	ApplyRootMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
}

void UCharacterSVMovementComponent::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, 
	AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Other->ActorHasTag("climb_top") && bClimbMode)
	{
		bClimbing = false;
		
		//---------------------------------------------------
		ACharacterSV* sv = Cast<ACharacterSV>(CharacterOwner);
		if (!sv)
			return;

		bClimbMode = false;
		sv->AbilityTouchOff(EAbilityBar::Climb, true);
	}
	else if (Other->ActorHasTag("climb_area"))
	{
		bFirstHit = false;
	}
	else if (Other->ActorHasTag("safe_house"))
	{
		ACharacterSV* pawn = Cast<ACharacterSV>(CharacterOwner);
		if (pawn) {
			pawn->bInSafeHouse = true;
		}
	}
	else if (Other->ActorHasTag("no_collision"))
	{
		ACharacterSV* pawn = Cast<ACharacterSV>(CharacterOwner);
		if (pawn)
		{
			if (pawn->IsMob() && !pawn->bFallingCollisionMode)
			{
				pawn->bNoCollisionMode = true;
				pawn->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
				pawn->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel9, ECR_Overlap);
			}
		}
	}
}

void UCharacterSVMovementComponent::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Black, "OnComponentEndOverlap");

	if (OtherActor->ActorHasTag("climb_area"))
	{
		bFirstHit = false;
	}
	else if (OtherActor->ActorHasTag("safe_house"))
	{
		ACharacterSV* pawn = Cast<ACharacterSV>(CharacterOwner);
		if (pawn) {
			pawn->bInSafeHouse = false;
		}
	}
	else if (OtherActor->ActorHasTag("no_collision"))
	{
		ACharacterSV* pawn = Cast<ACharacterSV>(CharacterOwner);
		if (pawn && pawn->bNoCollisionMode)
		{
			pawn->bNoCollisionMode = false;
			if (pawn->IsMob() && !pawn->bFallingCollisionMode)
			{
				pawn->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				pawn->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel9, ECR_Block);
			}
		}
	}
}

void UCharacterSVMovementComponent::OnComponentHit(UPrimitiveComponent* HitComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor->ActorHasTag("climb_hit") && !bFirstHit)
	{
		bFirstHit = true;
		bClimbMode = true;
		bClimbing = true;
		preVelocity = Velocity;
		//--------------------------------------------------------------
		if (Hit.bBlockingHit)
		{
			FVector impact = Hit.ImpactNormal * -1;
			impact.Normalize();
			//FRotator newRotator = CharacterOwner->GetActorRotation();
			//FString aaa = FString::Printf(TEXT("  x=%d y=%d z=%d \n"), impact.Rotation().Pitch, impact.Rotation().Yaw, impact.Rotation().Roll);
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, aaa);
			
			CharacterOwner->SetActorRotation(impact.Rotation());
		}
		//--------------------------------------------------------------
		SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climbing);
	}
	
}

void UCharacterSVMovementComponent::ResetMovementMode()
{
	 Velocity = preVelocity;
	SetMovementMode(MOVE_Walking);
	
}
