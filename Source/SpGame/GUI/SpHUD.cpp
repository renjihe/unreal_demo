// Fill out your copyright notice in the Description page of Project Settings.

#include "SpHUD.h"
#include "UObject/ConstructorHelpers.h"

#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#define AIM_ICON_WIDTH 512
ASpHUD::ASpHUD(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> aimIconTexture(TEXT("/Game/Artists/GUI/Aim"));
	if (aimIconTexture.Object) {
		AimIcon = aimIconTexture.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> aimRangeTexture(TEXT("/Game/Artists/GUI/AimRange"));
	if (aimRangeTexture.Object) {
		AimRange = aimRangeTexture.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> aimTipTexture(TEXT("/Game/Artists/GUI/AimTip"));
	if (aimTipTexture.Object) {
		AimTip = aimTipTexture.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> progressMaterial(TEXT("/Game/Blueprints/GUI/EnergyBar"));
	if (progressMaterial.Object) {
		EnergyBar = UMaterialInstanceDynamic::Create(progressMaterial.Object, this, FName(TEXT("EnergyBar")));
	}

	bResetTip = false;
	bShowEnergyBar = false;
}

void ASpHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
		return;

	ClipY = Canvas->ClipY;

	float scale = ClipY * 0.2f/ 1080.0f;
	float offset = AIM_ICON_WIDTH * scale / 2;
	DrawTextureSimple((UTexture *)AimRange, StartX - offset, StartY - offset, scale, false);
	if (EnergyBar && bShowEnergyBar) {
		Canvas->K2_DrawMaterial(EnergyBar, EnergyBarPos - FVector2D(0, EnergyBarSize.Y * scale), EnergyBarSize * scale, FVector2D(0, 0));
	}

	scale /= 2;
	offset /= 2;
	DrawTextureSimple((UTexture *)AimIcon, StartX - offset, StartY - offset, scale, false);
	
	if (!bResetTip) {
		DrawTextureSimple((UTexture *)AimTip, TipStartX - offset, TipStartY - offset, scale, false);
	}
}

void ASpHUD::UpdatePos(FVector2D pos)
{
	StartX = pos.X;
	StartY = pos.Y;
}

void ASpHUD::ResetTipPos()
{
	bResetTip = true;
}

void ASpHUD::UpdateTipPos(FVector2D pos)
{
	bResetTip = false;
	TipStartX = pos.X;
	TipStartY = pos.Y;
}

void ASpHUD::SetEnergyPercent(float percent)
{
	if (EnergyBar) {
		EnergyBar->SetScalarParameterValue(FName("Range"), percent);
	}
}

void ASpHUD::SetEnergyColor(FVector color)
{
	if (EnergyBar) {
		EnergyBar->SetVectorParameterValue(FName("Color"), color);
	}
}

void ASpHUD::UpdateEnergyBarPos(FVector2D screenPosition, FVector2D screenSize)
{
	EnergyBarPos = screenPosition;
	EnergyBarSize = screenSize;
}

void ASpHUD::ShowEnergyBar(bool bShow)
{
	bShowEnergyBar = bShow;	
}