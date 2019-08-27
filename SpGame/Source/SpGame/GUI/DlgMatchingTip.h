// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgMatchingTip.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class SPGAME_API UDlgMatchingTip : public UDlgSingleton
{
	GENERATED_BODY()
	
	DECLARE_DLGBASE_INSTANCE(UDlgMatchingTip)

protected:
	virtual void Create() override;
	virtual void Init() override;

	UButton				*BtnCancel;
	UTextBlock			*Timer;

	UFUNCTION()
	void BtnCancelClicked();
};
