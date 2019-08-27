 // Fill out your copyright notice in the Description page of Project Settings.

#include "DlgBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "GUI/DlgMgr.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

#include "Input/Events.h"

#include "client/dllclient.h"

UWorld* UDlgBase::World = nullptr;
UDlgBase* UDlgBase::TopChain = nullptr;

UDlgBase::UDlgBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ChainFrom = nullptr;
	bIsInChain = false;

	WidgetClass = nullptr;
	Widget = nullptr;	
	BtnClose = nullptr;
	bProcessEsc = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AddToRoot();
	}
}

void UDlgBase::ChainDlg(UDlgBase *dlg)
{
	//如果DlgBase为第一个窗口，则设置TopChain
	if (!GetTopChainDlg()) {
		TopChain = this;
		bIsInChain = true;
	}

	if (nullptr == dlg || dlg == this || dlg->bIsInChain) {
		return;
	}

	if (!bIsInChain)
		return;	

	UDlgBase* currDlg = GetTopChainDlg();
	while (currDlg != this)
	{
		currDlg->ChainFrom;
		Destroy();
	}

	// 把dlg加入到chain中
	dlg->ChainFrom = this;
	dlg->bIsInChain = true;
	dlg->Show();
	TopChain = dlg;
}

void UDlgBase::Destroy()
{
	SafeDestroy();
	DlgMgr->DelDlg(this);
}

void UDlgBase::SafeDestroy()
{
	Hide();
	//出栈
	TopChain = ChainFrom;
	if (IsRooted()) {
		RemoveFromRoot();
	}
}

void UDlgBase::Create(const FString& widget, bool processEsc)
{
	FString loadPath = "WidgetBlueprint'/Game/Blueprints/GUI/" + widget + "." + widget + "_C'";
	if (nullptr == WidgetClass) {
		WidgetClass = LoadClass<UUserWidget>(GetCurrWorld(), *loadPath);
	}

	if (nullptr == Widget) {
		Widget = CreateWidget<UUserWidget>(GetCurrWorld(), WidgetClass);
	}

	if (Widget)
	{
		BtnClose = (UButton *)GetChild("BtnClose");
		if (BtnClose) {
			BtnClose->OnClicked.AddDynamic(this, &UDlgBase::BtnCloseClicked);
		}

		LocalizationGUI(Widget);
	}

	Init();

	DlgMgr->AddDlg(this);

	bProcessEsc = processEsc;
}

void UDlgBase::Create()
{
}

void UDlgBase::Init()
{
}

void UDlgBase::Show(int32 ZOrder)
{
	if (Widget && !Widget->IsInViewport()) {
		Widget->AddToViewport(ZOrder);
	}
}

void UDlgBase::Hide()
{
	if (Widget && Widget->IsInViewport()) {
		Widget->RemoveFromParent();
	}
}

bool UDlgBase::PreprocessedInputKey(const FKey Key, const EInputEvent Event)
{
	bool bWasHandled = false;
	if (bProcessEsc && Key == EKeys::Escape && Event == IE_Pressed)
	{
		Destroy();
		bWasHandled = true;
	}
	return bWasHandled;
}

UWidget* UDlgBase::GetChild(const FString& widgetName)
{
	return Widget ? Widget->GetWidgetFromName(FName(*widgetName)) : nullptr;
}

void UDlgBase::LocalizationGUI(UUserWidget* userWidget)
{
	userWidget->WidgetTree->ForEachWidget([&userWidget, this](UWidget* widget) {
		if (widget->IsA(UUserWidget::StaticClass())) {
			this->LocalizationGUI(Cast<UUserWidget>(widget));
		}
		if (widget->IsA(UTextBlock::StaticClass())) {
			FText defaultText = Cast<UTextBlock>(widget)->GetText();
			FText resultText = FText::FromString(UTF8_TO_TCHAR(spclient::getString(TCHAR_TO_UTF8(*defaultText.ToString()))));

			Cast<UTextBlock>(widget)->SetText(resultText.IsEmpty() ? defaultText : resultText);
		}
	});
}

void UDlgBase::BtnCloseClicked()
{
	Destroy();
}
