// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "AnimInstanceBase.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
namespace EAnimAction
{
	enum Type
	{
		Reload,
		Attack,
		Under_Attack,
		Climb,
	};
}

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintThreadSafe))
class SVGAME_API UAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
	
	
public:
	UFUNCTION(BlueprintCallable, Category = "Anim|Custom")
	UAnimMontage* PlayAction(EAnimAction::Type AnimAction);

	UFUNCTION(BlueprintNativeEvent, Category = "Anim|Custom")
	UAnimMontage* Reload();

	UFUNCTION(BlueprintNativeEvent, Category = "Anim|Custom")
	UAnimMontage* Attack();

	UFUNCTION(BlueprintNativeEvent, Category = "Anim|Custom")
	UAnimMontage* UnderAttack();

	UFUNCTION(BlueprintNativeEvent, Category = "Anim|Custom")
	UAnimMontage* Climb();
	
};
