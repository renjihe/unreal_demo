// Fill out your copyright notice in the Description page of Project Settings.

#include "SpScrollBox.h"
#include "SpUserWidget.h"

USpScrollBox::USpScrollBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USpScrollBox::OnSelected(UUserWidget* spUserWidget, UWidget* widget)
{
	OnSelectedEvent.Broadcast(spUserWidget, widget);
}

void USpScrollBox::ScrollTo(int32 index, bool bAnimateScroll, EDescendantScrollDestination EScrollDestination)
{
	UWidget* Widget = GetChildAt(index);
	ScrollWidgetIntoView(Widget, bAnimateScroll, EScrollDestination);
}

void USpScrollBox::OnSlotAdded(UPanelSlot* InSlot)
{
	Super::OnSlotAdded(InSlot);
		
	if (USpUserWidget* item = Cast<USpUserWidget>(InSlot->Content))	{
		item->OnSelected.AddDynamic(this, &USpScrollBox::OnSelected);
	}	
}