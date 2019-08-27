// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

class FSpGameModule : public FDefaultGameModuleImpl
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	//virtual void PostLoadCallback() override;
	//virtual void PreUnloadCallback() override;
	virtual void ShutdownModule() override;
};

extern UWorldProxy GSpWorld;
extern void SwitchInputMode(bool bGuiMode, bool bGuiOnly = false);
extern void GetSprayColor(FVector &AtkerColor, FVector &DeferColor);
extern FVector GetSprayColor(int faction);
extern FName GetAvatarName(AvatarPosition pos);
