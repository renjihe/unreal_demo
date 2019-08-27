// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GridPanel.h"
#include "SpGridPanel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridPanelDelegate, UUserWidget*, SpUserWidget, UWidget*, Widget);

/**
 * 
 */
UCLASS()
class SPGAME_API USpGridPanel : public UGridPanel
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sp Grid Panel")
	int32 ColumnNum;

	UFUNCTION()
	void OnSelected(UUserWidget* spUserWidget, UWidget* widget);

	FGridPanelDelegate OnSelectedEvent;

protected:
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
};
