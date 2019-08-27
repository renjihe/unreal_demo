// Fill out your copyright notice in the Description page of Project Settings.

#include "SpToolTip.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"

USpToolTip::USpToolTip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USpToolTip::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{	
	During -= InDeltaTime;
	if (0 > During) {
		RemoveFromParent();
	}

	FVector2D translation = RenderTransform.Translation + Direct * Rate;
	SetRenderTranslation(translation);
	ColorAndOpacity.A -= ColorAndOpacity.A * InDeltaTime / During;
	SetColorAndOpacity(ColorAndOpacity);	
}

void USpToolTip::ShowToolTip(UWidget *targetWidget, FString message, int32 fontSize, float rate, float during, EDirection direction)
{
	FVector2D targetPos = FVector2D(960, 540);
	if (targetWidget) {
		targetPos = targetWidget->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.5, 0.5));
	}

	//Inherits UPanelWidget->UScaleBox->USizeBox->UPanelWidget->UTextBlock
	USpToolTip *newToolTip = CreateWidget<USpToolTip>(GSpWorld, USpToolTip::StaticClass());

	UPanelWidget *rootWidget = newToolTip->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), FName(TEXT("CanvasPanel")));
	newToolTip->WidgetTree->RootWidget = rootWidget;
	rootWidget->Visibility = ESlateVisibility::HitTestInvisible;

	UScaleBox *scaleBox = newToolTip->WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), FName(TEXT("scb")));
	rootWidget->AddChild(scaleBox);

	UCanvasPanelSlot *scaleBoxSlot = Cast<UCanvasPanelSlot>(scaleBox->Slot);
	scaleBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.f, 1.f));
	scaleBoxSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 0.f));
	scaleBoxSlot->SetAlignment(FVector2D(.5f, .5f));

	USizeBox *sizeBox = newToolTip->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), FName(TEXT("sib")));
	sizeBox->SetHeightOverride(1080);
	sizeBox->SetWidthOverride(1920);
	scaleBox->AddChild(sizeBox);

	UPanelWidget *panelWidget = newToolTip->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), FName(TEXT("CanvasPanel2")));
	sizeBox->AddChild(panelWidget);

	UTextBlock* textWidget = newToolTip->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(TEXT("Message")));
	textWidget->SetText(FText::FromString(message));
	textWidget->Font.Size = fontSize;
	panelWidget->AddChild(textWidget);

	UCanvasPanelSlot *textSlot = Cast<UCanvasPanelSlot>(textWidget->Slot);
	textSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	textSlot->SetOffsets(FMargin(targetPos.X, targetPos.Y, 0.f, 0.f));
	textSlot->SetAlignment(FVector2D(.5f, .5f));
	textSlot->SetAutoSize(true);
	
	newToolTip->Rate = rate;
	newToolTip->During = during;
	switch (direction)
	{
	case EDirection::Up:
		newToolTip->Direct = FVector2D(0.f, -1.f);
		break;
	case EDirection::Down:
		newToolTip->Direct = FVector2D(0.f, 1.f);
		break;
	case EDirection::Left:
		newToolTip->Direct = FVector2D(-1.f, 0.f);
		break;
	case EDirection::Right:
		newToolTip->Direct = FVector2D(1.f, 0.f);
		break;
	default:
		newToolTip->Direct = FVector2D(0.f, 0.f);
	}

	newToolTip->AddToViewport(10);
}