// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpUserWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelectDelegate, UUserWidget*, SpUserWidget, UWidget*, Widget);

/**
 * 
 */
UCLASS()
class SPGAME_API USpUserWidget : public UUserWidget
{
	GENERATED_UCLASS_BODY()
	
public:
	void NativeOnSelected(UWidget* widget);

	FSelectDelegate OnSelected;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
