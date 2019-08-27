// Fill out your copyright notice in the Description page of Project Settings.

#include "MsgBox.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Button.h"

#include "client/dllclient.h"

UMsgBox::UMsgBox(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
	BtnYes = nullptr;
	BtnCancel = nullptr;
	Title = nullptr;
	Content = nullptr;
}

UMsgBox* UMsgBox::CreateMsgBox(FString title, FString content, MSGBOX_CALLBACK func, EMsgBoxFlag flag)
{
	UMsgBox* newMsg = NewObject<UMsgBox>(GetCurrWorld(), UMsgBox::StaticClass());

	newMsg->Create();

	if (newMsg->Widget)
	{
		newMsg->Title = (UTextBlock *)newMsg->GetChild("title");
		if (newMsg->Title) {
			newMsg->Title->SetText(FText::FromString(UTF8_TO_TCHAR(spclient::getString(TCHAR_TO_UTF8(*title)))));
		}

		newMsg->Content = (UTextBlock *)newMsg->GetChild("content");
		if (newMsg->Content) {
			newMsg->Content->SetText(FText::FromString(UTF8_TO_TCHAR(spclient::getString(TCHAR_TO_UTF8(*content)))));
		}

		newMsg->CallbackFunc = func;
		
		newMsg->BtnYes = (UButton *)newMsg->GetChild("BtnYes");
		if (newMsg->BtnYes)
		{
			newMsg->BtnYes->SetVisibility((flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Ok) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			newMsg->BtnYes->OnClicked.AddDynamic(newMsg, &UMsgBox::BtnYesClicked);
		}

		newMsg->BtnCancel = (UButton *)newMsg->GetChild("BtnCancel");
		if (newMsg->BtnCancel)
		{
			newMsg->BtnCancel->SetVisibility((flag == EMsgBoxFlag::OkCancel || flag == EMsgBoxFlag::Cancel) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			newMsg->BtnCancel->OnClicked.AddDynamic(newMsg, &UMsgBox::BtnCancelClicked);
		}		
	}

	newMsg->Show();
	return newMsg;
}

void UMsgBox::Create()
{
	UDlgBase::Create("Message");
}

bool UMsgBox::PreprocessedInputKey(const FKey Key, const EInputEvent Event)
{
	if (Super::PreprocessedInputKey(Key, Event))
	{
		if (CallbackFunc) {
			CallbackFunc(1);
		}
		return true;
	}

	return false;
}

void UMsgBox::BtnYesClicked()
{
	if (CallbackFunc) {
		CallbackFunc(0);
	}

	Destroy();
}

void UMsgBox::BtnCancelClicked()
{
	if (CallbackFunc) {
		CallbackFunc(1);
	}

	Destroy();
}