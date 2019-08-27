// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilityProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"


AAbilityProjectile::AAbilityProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->Activate(false);

	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AAbilityProjectile::OnProjectileStop);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AAbilityProjectile::OnComponentBeginOverlap);
	DelayTouchOff = 0.0f;
}

void AAbilityProjectile::ActivateProjectile(EAbilityProjectile::Type projectiletype)
{
	switch (projectiletype)
	{
	case EAbilityProjectile::Homing:
		ProjectileType = EAbilityProjectile::Homing;
		break;
	case EAbilityProjectile::Spline:
		ProjectileType = EAbilityProjectile::Spline;
		break;
	default:
		break;
	}
	ProjectileMovement->Activate(true);
}

void AAbilityProjectile::SetProjectileVelocity(FVector projectileVelocity, float time)
{
	ProjectileMovement->Velocity = projectileVelocity;

	DelayTouchOff = time;
	if ( DelayTouchOff > 0.001) {
		GWorld->GetTimerManager().SetTimer(DelayTouchOffTimer, this, &AAbilityProjectile::AbilityDelayTouchOff, DelayTouchOff, false);
	}
}

void AAbilityProjectile::OnProjectileStop(const FHitResult& hit)
{
	if (Role != ROLE_Authority) {
		return;
	}

	switch (ProjectileType)
	{
	case EAbilityProjectile::Homing:
		break;
	case EAbilityProjectile::Spline:
		{
			UCapsuleComponent* capsule = GetCapsuleComponent();
			FVector velocity = capsule->GetPhysicsLinearVelocity();
			velocity.Normalize();
			capsule->SetPhysicsLinearVelocity(velocity * 200);
			capsule->SetSimulatePhysics(true);

			if (DelayTouchOff < 0.001) {
				AbilityDelayTouchOff();
			}
		}
		break;
	default:
		break;
	}
	
}

void AAbilityProjectile::AbilityEnded(int abilityID)
{
	if (abilityID == AbilityList[0]) {
		Destroy();
	}
}

void AAbilityProjectile::AbilityDelayTouchOff()
{
	GWorld->GetTimerManager().ClearTimer(DelayTouchOffTimer);

	TryActivateAbilityByIndex(0);
}

void AAbilityProjectile::ActivationTraceMode(ACharacterSV* target)
{
	if (target)
	{
		ProjectileMovement->HomingTargetComponent = target->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = 1500;
		ProjectileMovement->bIsHomingProjectile = true;
	}
}

void AAbilityProjectile::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Role != ROLE_Authority) {
		return;
	}

	switch (ProjectileType)
	{
	case EAbilityProjectile::Homing:
		{
			AMajorCharacter* mc = Cast<AMajorCharacter>(Other);
			if (mc){
				TryActivateAbilityByIndex(EAbilityBar::BulletHP, Other);
			}
			Destroy(true);
		}
		break;
	case EAbilityProjectile::Spline:
		break;
	default:
		break;
	}
	
}
