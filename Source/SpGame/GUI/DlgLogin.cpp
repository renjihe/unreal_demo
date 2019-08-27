// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgLogin.h"
#include "GUI/MsgBox.h"
#include "REs/GlobalConfigRes.h"

#include "client/dllclient.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgLogin)

void UDlgLogin::Create()
{
	UDlgBase::Create("Login", false);
}

void UDlgLogin::Init()
{
	if (Widget)
	{
		TextUserName = (UEditableText *)GetChild("UserName");
		if (TextUserName) {
			TextUserName->OnTextCommitted.AddDynamic(this, &UDlgLogin::OnTextCommitted);
		}

		TextPassWord = (UEditableText *)GetChild("PassWord");
		if (TextPassWord) {
			TextPassWord->OnTextCommitted.AddDynamic(this, &UDlgLogin::OnTextCommitted);
		}

		BtnSubmit = (UButton *)GetChild("BtnSubmit");
		if (BtnSubmit) {
			BtnSubmit->OnClicked.AddDynamic(this, &UDlgLogin::BtnSubmitClicked);
		}

		BtnTest = (UButton *)GetChild("BtnTest");
		if (BtnTest) {
			BtnTest->OnClicked.AddDynamic(this, &UDlgLogin::BtnTestClicked);
		}
	}
}
extern void LoadMap(const char *path);
void UDlgLogin::BtnSubmitClicked()
{
	FString userName = "";
	FString passWord = "";
	if (TextUserName)
		userName = TextUserName->GetText().ToString();

	if (TextPassWord)
		passWord = TextPassWord->GetText().ToString();

	if (userName.IsEmpty())
	{
		UMsgBox::CreateMsgBox(TEXT("Notice"), TEXT("user name can't be empty!"), NULL, EMsgBoxFlag::Ok);
		return;
	}

	const FGlobalConfigRow &config = FGlobalConfigRes::Get().GetConfig(TEXT("ServerURL"));

	FString serverUrl = config.Desc;
	if (serverUrl.IsEmpty()) {
		serverUrl = "192.168.0.6";
	}

	int port = config.IntValue.Num() > 0 ? config.IntValue[0] : 0;
	if (port == 0) {
		port = 8901;
	}

	spclient::login(TCHAR_TO_UTF8(*userName), TCHAR_TO_UTF8(*passWord), TCHAR_TO_UTF8(*serverUrl), port);
}

void UDlgLogin::BtnTestClicked()
{
	LoadMap("/Game/Maps/Campsite");
}

void UDlgLogin::OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter) {
		BtnSubmitClicked();
	}
}