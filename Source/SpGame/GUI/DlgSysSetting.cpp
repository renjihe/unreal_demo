// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgSysSetting.h"
#include "SpGame.h"
#include "GUI/MsgBox.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/EditableText.h"

#include "client/dllclient.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgSysSetting)

void UDlgSysSetting::Create()
{
	UDlgBase::Create("SysSetting");
}

void UDlgSysSetting::Init()
{
	if (Widget)
	{
		BtnGiveUp = (UButton *)GetChild("BtnGiveUp");
		if (BtnGiveUp) {
			BtnGiveUp->OnClicked.AddDynamic(this, &UDlgSysSetting::BtnGiveUpClicked);
		}
	}
}

void UDlgSysSetting::BtnGiveUpClicked()
{
	Destroy();
	spclient::giveupDuelReal();
}

void UDlgSysSetting::Destroy()
{
	SwitchInputMode(false);
	Super::Destroy();
}