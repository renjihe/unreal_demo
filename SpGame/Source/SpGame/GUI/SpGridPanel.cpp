// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGridPanel.h"
#include "SpUserWidget.h"

#include "Components/GridSlot.h"

USpGridPanel::USpGridPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ColumnNum = 3;
}

void USpGridPanel::OnSelected(UUserWidget* spUserWidget, UWidget* widget)
{
	OnSelectedEvent.Broadcast(spUserWidget, widget);
}

void USpGridPanel::OnSlotAdded(UPanelSlot* Slot)
{
	Super::OnSlotAdded(Slot);

	int32 index = GetChildrenCount() - 1;
	UGridSlot* insertSlot = Cast<UGridSlot>(Slot);
	if (insertSlot)
	{
		insertSlot->SetColumn((index) % ColumnNum);
		insertSlot->SetRow((index) / ColumnNum);
	}	

	USpUserWidget* item = Cast<USpUserWidget>(Slot->Content);
	if (item) {
		item->OnSelected.AddDynamic(this, &USpGridPanel::OnSelected);
	}
}

void USpGridPanel::OnSlotRemoved(UPanelSlot* Slot)
{
	Super::OnSlotRemoved(Slot);
	
	for (int32 i = 0; i < GetChildrenCount(); ++i)
	{
		UGridSlot* slot = Cast<UGridSlot>(Slots[i]);
		slot->SetColumn(i % ColumnNum);
		slot->SetRow(i / ColumnNum);
	}
}