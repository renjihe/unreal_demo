// Fill out your copyright notice in the Description page of Project Settings.

#include "SpUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "SpButton.h"

USpUserWidget::USpUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Visibility = ESlateVisibility::Visible;
}

void USpUserWidget::NativeConstruct()
{
	WidgetTree->ForEachWidget([this](UWidget* widget) {
		if (USpButton* spButton = Cast<USpButton>(widget)) {
			spButton->SetOwner(this);
		}
	});
}

FReply USpUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSelected.Broadcast(this, nullptr);

	return FReply::Handled();
}

void USpUserWidget::NativeOnSelected(UWidget* widget)
{
	OnSelected.Broadcast(this, widget);
}