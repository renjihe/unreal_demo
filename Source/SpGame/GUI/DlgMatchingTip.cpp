// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgMatchingTip.h"

#include "client/dllclient.h"

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Runtime/UMG/Public/Components/Button.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgMatchingTip)

void UDlgMatchingTip::Create()
{
	UDlgBase::Create("MatchingWaiting");
}

void UDlgMatchingTip::Init()
{
	if (Widget)
	{
		Timer = (UTextBlock *)GetChild("Timer");

		BtnCancel = (UButton *)GetChild("BtnCancel");
		if (BtnCancel) {
			BtnCancel->OnClicked.AddDynamic(this, &UDlgMatchingTip::BtnCancelClicked);
		}
	}
}

void UDlgMatchingTip::BtnCancelClicked()
{
	spclient::cancelDuelRealMatch();
	Destroy();
}