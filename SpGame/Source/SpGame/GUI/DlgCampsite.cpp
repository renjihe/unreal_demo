// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgCampsite.h"
#include "DlgMatchingTip.h"

#include "client/dllclient.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgCampsite)

void UDlgCampsite::Create()
{
	UDlgBase::Create("Campsite", false);
}

void UDlgCampsite::Init()
{
	if (Widget)
	{
		UserName = (UTextBlock *)GetChild("UserName");
		if (UserName) {
			UserName->SetText(FText::FromString(UTF8_TO_TCHAR(spclient::getPlayerName())));
		}

		BtnMatch = (UButton *)GetChild("BtnMatch");
		if (BtnMatch) {
			BtnMatch->OnClicked.AddDynamic(this, &UDlgCampsite::BtnMatchClicked);
		}

		QualifyingMatch = (UButton *)GetChild("QualifyingMatch");
		if (QualifyingMatch) {
			QualifyingMatch->OnClicked.AddDynamic(this, &UDlgCampsite::QualifyingMatchClicked);
		}
	}
}

void UDlgCampsite::BtnMatchClicked()
{
	spclient::startPVE("/Game/Maps/DemoTest");
}

void UDlgCampsite::QualifyingMatchClicked()
{
	const char *name = spclient::getPlayerName();
	if (name && name[0])
	{
		if (!UDlgMatchingTip::IsInstanced())
		{
			spclient::startDuelRealMatch();
			UDlgMatchingTip::GetInstance()->Show(255);
		}

		//BtnMatch->SetIsEnabled(false);
		UDlgCampsite::GetInstance()->ChainDlg(UDlgMatchingTip::GetInstance());
	}
}