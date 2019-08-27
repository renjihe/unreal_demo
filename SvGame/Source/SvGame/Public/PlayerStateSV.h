// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "Abilities/AbilitiesType.h"
#include "PlayerStateSV.generated.h"

/**
 * 
 */
UCLASS()
class SVGAME_API APlayerStateSV : public APlayerState
{
	GENERATED_UCLASS_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UPROPERTY(Transient, Replicated)
	int32 LookId;

	UPROPERTY(Transient, Replicated)
	int32 WeaponId;

	int32 PawnId;

	bool bInitAttrib;
	FAttribs Attrib;
protected:
	virtual void CopyProperties(APlayerState* PlayerState);

	TEnumAsByte<EStateType::Type> PlayState;
};
