// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DlgBase.generated.h"

enum EInputEvent;
struct FKey;
class UClass;
class UWidget;
class UUserWidget;
class UButton;
class FDlgMgr;
class UWidgetTree;

/**
 * 
 */
UCLASS()
class SPGAME_API UDlgBase : public UObject
{
	GENERATED_UCLASS_BODY()

	friend class FDlgMgr;
public:
	static void SetCurrWorld(UWorld* inWorld) { World = inWorld; }
	static UWorld* GetCurrWorld() { return World; }
protected:
	static UWorld	*World;

	bool bProcessEsc;
public:
	void ChainDlg(UDlgBase *dlg);
	virtual void Destroy();
protected:
	virtual void SafeDestroy();
	static UDlgBase* GetTopChainDlg() { return TopChain; };
private:
	UDlgBase		*ChainFrom;
	bool			bIsInChain;
	static UDlgBase *TopChain;
	
protected:
	void Create(const FString& widget, bool processEsc = true);
public:
	virtual void Create();
	virtual void Init();
	void Show(int32 ZOrder = 0);
	void Hide();

	virtual bool PreprocessedInputKey(const FKey Key, const EInputEvent Event);
protected:
	UWidget* GetChild(const FString& widgetName);
	void LocalizationGUI(UUserWidget* widget);
	UFUNCTION()
	void BtnCloseClicked();	

protected:
	UClass			*WidgetClass;
	UUserWidget		*Widget;
	UButton			*BtnClose;
};
