// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "SpHUD.generated.h"

class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class SPGAME_API ASpHUD : public AHUD
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void DrawHUD();
	void UpdatePos(FVector2D pos);
	void ResetTipPos();

	void UpdateTipPos(FVector2D pos);
	void UpdateEnergyBarPos(FVector2D screenPosition, FVector2D screenSize);
	void SetEnergyPercent(float percent);
	void SetEnergyColor(FVector color);
	void ShowEnergyBar(bool bShow);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpHUD")
	UTexture2D *AimIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpHUD")
	UTexture2D *AimRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpHUD")
	UTexture2D *AimTip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpHUD")
	UMaterialInstanceDynamic *EnergyBar;

private:
	float ClipX;
	float ClipY;
	float StartX;
	float StartY;

	bool  bResetTip;
	float TipStartX;
	float TipStartY;

	bool  bShowEnergyBar;
	FVector2D EnergyBarPos;
	FVector2D EnergyBarSize;
};