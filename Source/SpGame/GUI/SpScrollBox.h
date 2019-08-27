// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "SpScrollBox.generated.h"

class UUserWidget;
class UWidget;
class UPanelSlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScrollBoxDelegate, UUserWidget*, SpUserWidget, UWidget*, Widget);

/**
 * 
 */
UCLASS()
class SPGAME_API USpScrollBox : public UScrollBox
{
	GENERATED_UCLASS_BODY()
	
public:
	FScrollBoxDelegate OnSelectedEvent;

	UFUNCTION()
	void OnSelected(UUserWidget* spUserWidget, UWidget* widget);
	void ScrollTo(int32 index, bool bAnimateScroll = true, EDescendantScrollDestination EScrollDestination = EDescendantScrollDestination::TopOrLeft);

protected:
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;	
	
};
