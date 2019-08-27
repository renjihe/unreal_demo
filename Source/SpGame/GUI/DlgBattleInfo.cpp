// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgBattleInfo.h"
#include "GUI/MsgBox.h"
#include "client/dllclient.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Styling/SlateBrush.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgBattleInfo)

void UDlgBattleInfo::Create()
{
	UDlgBase::Create("Battle", false);
}

void UDlgBattleInfo::Init()
{
	MpProgressBar = nullptr;

	if (Widget)
	{
		BattleTimer = (UTextBlock *)GetChild("BattleTimer");
		RebirthTimer = (UTextBlock *)GetChild("RebirthTimer");

		HpTextBlock = (UTextBlock *)GetChild("HpTextBlock");
		MpTextBlock = (UTextBlock *)GetChild("MpTextBlock");

		DeadCanvas = (UWidget *)GetChild("DeadCanvas");
		if (DeadCanvas) {
			DeadCanvas->SetVisibility(ESlateVisibility::Hidden);
		}

		UImage *MiniMap = (UImage *)GetChild("MiniMap");
		if (MiniMap)
		{
		}

		UImage *progressImage = (UImage *)GetChild("ProgressBar");
		if (progressImage)
		{
			UMaterialInterface *mat = Cast<UMaterialInterface>(progressImage->Brush.GetResourceObject());			
			if (mat)
			{
				MpProgressBar = Cast<UMaterialInstanceDynamic>(mat);
				if (MpProgressBar == nullptr) {
					MpProgressBar = UMaterialInstanceDynamic::Create(mat, this);
				}

				MpProgressBar->SetScalarParameterValue(TEXT("Range"), 0.f);
				progressImage->Brush.SetResourceObject(MpProgressBar);				
			}		
		}

		UUniformGridPanel *SprayPanel = (UUniformGridPanel *)GetChild("SprayPanel");
		if (SprayPanel)
		{
			//SprayPanel->AddChildToUniformGrid
		}

	}
}

static FString TimeSecondsToString(int seconds)
{
	const int32 NumMinutes = FMath::FloorToInt(seconds / 60.f);
	const int32 NumSeconds = FMath::FloorToInt(seconds - (NumMinutes * 60.f));
	return FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
}

void UDlgBattleInfo::UpdateFrame(float DeltaSeconds)
{
	int frame = (5400 - spclient::getBattleFrame())/30;
	if (BattleTimer) {
		BattleTimer->SetText(FText::FromString(TimeSecondsToString(frame)));
	}
	
	if (RebirthTimer) {
		RebirthTimer->SetText(FText::FromString(TimeSecondsToString(spclient::getRebirthTime()/30)));
	}

	if (HpTextBlock) {
		HpTextBlock->SetText(FText::FromString(("HP  " + FString::FromInt(spclient::getBattleUserAttrib(summer::ATTRIB_HP)))));
	}

	if (MpTextBlock) {
		MpTextBlock->SetText(FText::FromString(("MP  " + FString::FromInt(spclient::getBattleUserAttrib(summer::ATTRIB_MP)))));
	}
}

void UDlgBattleInfo::ShowDeadNotice()
{
	if (DeadCanvas) {
		DeadCanvas->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDlgBattleInfo::ShowRebirthNotice()
{
	if (DeadCanvas)	{
		DeadCanvas->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDlgBattleInfo::UpdateMPRange(float rate)
{
	if (MpProgressBar) {
		MpProgressBar->SetScalarParameterValue(TEXT("Range"), rate);
	}	
}