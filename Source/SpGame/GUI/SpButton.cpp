// Fill out your copyright notice in the Description page of Project Settings.

#include "SpButton.h"
#include "SpUserWidget.h"

USpButton::USpButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		OnClicked.AddDynamic(this, &USpButton::SlateHandleClicked);
	}
}

void USpButton::SlateHandleClicked()
{
	USpUserWidget* item = Cast<USpUserWidget>(Owner);
	item->NativeOnSelected(this);
}