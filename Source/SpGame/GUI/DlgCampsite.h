// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgCampsite.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class SPGAME_API UDlgCampsite : public UDlgSingleton
{
	GENERATED_BODY()
	
	DECLARE_DLGBASE_INSTANCE(UDlgCampsite)

protected:
	virtual void Create() override;
	virtual void Init() override;

	UButton				*BtnMatch;
	UButton				*QualifyingMatch;
	UTextBlock	*UserName;

	UFUNCTION()
	void BtnMatchClicked();

	UFUNCTION()
	void QualifyingMatchClicked();
};
