// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "SlateBasics.h"
#include "EditorStyleSet.h"
#include "AssetTypeActions_Base.h"

class FSpGameEditorCommands : public TCommands<FSpGameEditorCommands>
{
public:
	FSpGameEditorCommands()
		: TCommands<FSpGameEditorCommands>(TEXT("SpGameEditor"), FText::FromString("SpGame Editor")
						, NAME_None, FEditorStyle::GetStyleSetName()) 
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> Command;
	TSharedPtr<FUICommandInfo> Command_reserved;
};

class FSpGameEdModule : public FDefaultGameModuleImpl
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	//virtual void PostLoadCallback() override;
	//virtual void PreUnloadCallback() override;
	virtual void ShutdownModule() override;

	static void AddMenuEntry(class FMenuBuilder &menuBuilder);

	TSharedPtr<class FUICommandList> EditorCommands;
	TSharedPtr<FExtender> MenuExtender;

	TSharedPtr<FAssetTypeActions_Base> BinAssetTypeActions, SprayMaskTypeActions;
};
