// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpToolTip.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Up,
	Down,
	Left,
	Right,
	None
};

/**
 * 
 */
UCLASS()
class SPGAME_API USpToolTip : public UUserWidget
{
	GENERATED_UCLASS_BODY()
public:
	static void ShowToolTip(UWidget *targetWidget, FString message = "Default", int32 fontSize = 30, float rate = 0.1f, float during = 1.f, EDirection direction = EDirection::Up);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	float		Rate;
	float		During;
	FVector2D	Direct;
};

extern UWorldProxy GSpWorld;