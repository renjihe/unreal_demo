// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilityTask_PlayMontageAndWaitSV.h"
#include "Character/CharacterSV.h"
#include "AbilitySystemGlobals.h"

UAbilityTask_PlayMontageAndWaitSV::UAbilityTask_PlayMontageAndWaitSV(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bStopWhenAbilityEnds = true;
}

void UAbilityTask_PlayMontageAndWaitSV::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		OnInterrupted.Broadcast();
	}
	else
	{
		OnBlendOut.Broadcast();
	}
}

void UAbilityTask_PlayMontageAndWaitSV::OnMontageInterrupted()
{
	if (StopPlayingMontage())
	{
		// Let the BP handle the interrupt as well
		OnInterrupted.Broadcast();
	}
}

void UAbilityTask_PlayMontageAndWaitSV::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		OnCompleted.Broadcast();
	}

	EndTask();
}

UAbilityTask_PlayMontageAndWaitSV* UAbilityTask_PlayMontageAndWaitSV::CreatePlayMontageAndWaitProxy(UGameplayAbility* OwningAbility, FName TaskInstanceName, AActor *fromActor, 
	EEffectPosition::Type fromPosition, EAnimAction::Type animAction, bool bStopWhenAbilityEnds)
{

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	USkeletalMeshComponent* skeletalMesh = Cast<USkeletalMeshComponent>(mesh);

	UAbilityTask_PlayMontageAndWaitSV* MyObj = NewAbilityTask<UAbilityTask_PlayMontageAndWaitSV>(OwningAbility, TaskInstanceName);
	MyObj->SkeletalMesh = skeletalMesh;
	MyObj->FromPosition = fromPosition;
	MyObj->AnimAction = animAction;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

	return MyObj;
}

void UAbilityTask_PlayMontageAndWaitSV::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	if (SkeletalMesh)
	{
		ACharacterSV* ch = Cast<ACharacterSV>(AbilitySystemComponent->GetOwner());
		if (ch) {
			ch->NetMulticastPlayMontage(SkeletalMesh, AnimAction, FromPosition, this);
		}

	}

	SetWaitingOnAvatar();
}

void UAbilityTask_PlayMontageAndWaitSV::SetActionStart(UAnimMontage* montage)
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (SkeletalMesh && montage)
	{
		UAnimInstanceBase* AnimInstance = Cast<UAnimInstanceBase>(SkeletalMesh->GetAnimInstance());
		if (AnimInstance != nullptr)
		{
			MontageToPlay = montage;
			float Duration = MontageToPlay->GetPlayLength();

			if (Duration > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (IsPendingKill())
				{
					OnCancelled.Broadcast();
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayMontageAndWaitSV::OnMontageInterrupted);

				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitSV::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayMontageAndWaitSV::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitSV call to PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitSV called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWaitSV called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		OnCancelled.Broadcast();
	}
}

void UAbilityTask_PlayMontageAndWaitSV::ExternalCancel()
{
	check(AbilitySystemComponent);

	OnCancelled.Broadcast();
	Super::ExternalCancel();
}

void UAbilityTask_PlayMontageAndWaitSV::OnDestroy(bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	Super::OnDestroy(AbilityEnded);

}


bool UAbilityTask_PlayMontageAndWaitSV::StopPlayingMontage()
{
	if (!SkeletalMesh)
	{
		return false;
	}

	UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	if (AbilitySystemComponent && Ability)
	{
		
		if (AbilitySystemComponent->GetAnimatingAbility() == Ability
			&& Ability->GetCurrentMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			bool bShouldStopMontage = !AnimInstance->Montage_GetIsStopped(MontageToPlay);

			if (bShouldStopMontage)
			{
				ACharacterSV* ch = Cast<ACharacterSV>(AbilitySystemComponent->GetOwner());
				if (ch) {
					ch->NetMulticastStopMontage(SkeletalMesh, FromPosition);
				}
				//AnimInstance->Montage_Stop(MontageToPlay->BlendOut.GetBlendTime(), MontageToPlay);
			}

			return true;
		}
	}

	return false;
}

FString UAbilityTask_PlayMontageAndWaitSV::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (SkeletalMesh)
	{
		UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWait. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

