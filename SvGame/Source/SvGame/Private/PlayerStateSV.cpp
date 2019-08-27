// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "PlayerStateSV.h"
#include <UnrealNetwork.h>

APlayerStateSV::APlayerStateSV(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	//LookId = WeaponId = 0;
	bInitAttrib = false;
}


void APlayerStateSV::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerStateSV, LookId);
	DOREPLIFETIME(APlayerStateSV, WeaponId);
}

void APlayerStateSV::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	APlayerStateSV *PS = Cast<APlayerStateSV>(PlayerState);
	if (PS)
	{
		PS->LookId = LookId;
		PS->WeaponId = WeaponId;
		PS->PawnId = PawnId;
		PS->bInitAttrib = bInitAttrib;
		PS->Attrib = Attrib;
	}
}
