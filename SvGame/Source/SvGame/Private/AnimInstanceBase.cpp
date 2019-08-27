// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AnimInstanceBase.h"

UAnimMontage* UAnimInstanceBase::PlayAction(EAnimAction::Type AnimAction)
{
	switch (AnimAction)
	{
	case EAnimAction::Reload:
		return Reload();
	case EAnimAction::Attack:
		return Attack();
	case EAnimAction::Under_Attack:
		return UnderAttack();
	case EAnimAction::Climb:
		return Climb();
	default:
		return nullptr;
	}

	return nullptr;
}

UAnimMontage* UAnimInstanceBase::Reload_Implementation()
{
	return nullptr;
}

UAnimMontage* UAnimInstanceBase::Attack_Implementation()
{
	return nullptr;
}

UAnimMontage* UAnimInstanceBase::UnderAttack_Implementation()
{
	return nullptr;
}

UAnimMontage* UAnimInstanceBase::Climb_Implementation()
{
	return nullptr;
}