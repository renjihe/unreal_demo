// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "SpButton.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API USpButton : public UButton
{
	GENERATED_UCLASS_BODY()

public:
	void SetOwner(UUserWidget* owner) { Owner = owner; }
	const UUserWidget* GetOwner() const { return Owner; }

protected:
	UFUNCTION()
	void SlateHandleClicked();

private:
	UUserWidget* Owner;
};
