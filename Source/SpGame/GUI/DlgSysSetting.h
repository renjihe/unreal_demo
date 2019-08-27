// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgSysSetting.generated.h"

class UButton;
class UEditableText;
/**
 * 
 */
UCLASS()
class SPGAME_API UDlgSysSetting : public UDlgSingleton
{
	GENERATED_BODY()
	DECLARE_DLGBASE_INSTANCE(UDlgSysSetting)

protected:
	virtual void Create() override;
	virtual void Init() override;

	UButton				*BtnGiveUp;

	UFUNCTION()
	void BtnGiveUpClicked();
public:
	virtual void Destroy() override;
};
