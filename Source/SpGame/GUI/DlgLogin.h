// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgLogin.generated.h"

class UButton;
class UEditableText;
/**
 * 
 */
UCLASS()
class SPGAME_API UDlgLogin : public UDlgSingleton
{
	GENERATED_BODY()
	DECLARE_DLGBASE_INSTANCE(UDlgLogin)

protected:
	virtual void Create() override;
	virtual void Init() override;

	UButton			*BtnSubmit;
	UButton			*BtnTest;
	UEditableText	*TextUserName;
	UEditableText	*TextPassWord;

	UFUNCTION()
	void BtnSubmitClicked();

	UFUNCTION()
	void BtnTestClicked();

	UFUNCTION()
	void OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

};
