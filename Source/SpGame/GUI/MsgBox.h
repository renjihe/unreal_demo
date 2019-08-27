// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgBase.h"
#include "MsgBox.generated.h"

class UButton;
class UTextBlock;

UENUM(BlueprintType)
enum class EMsgBoxFlag : uint8
{
	OkCancel,
	Ok,
	Cancel,
	None
};

typedef void(*MSGBOX_CALLBACK)(int32 BtnIndex);

UCLASS()
class SPGAME_API UMsgBox : public UDlgBase
{
	GENERATED_UCLASS_BODY()
public:
	static UMsgBox* CreateMsgBox(FString title, FString content, MSGBOX_CALLBACK func, EMsgBoxFlag flag = EMsgBoxFlag::OkCancel);
	virtual void Create() override;
	virtual bool PreprocessedInputKey(const FKey Key, const EInputEvent Event) override;

protected:
	UButton *BtnYes;
	UButton *BtnCancel;
	UTextBlock *Title;
	UTextBlock *Content;

	UFUNCTION()
	void BtnYesClicked();
	UFUNCTION()
	void BtnCancelClicked();

	MSGBOX_CALLBACK CallbackFunc;
};