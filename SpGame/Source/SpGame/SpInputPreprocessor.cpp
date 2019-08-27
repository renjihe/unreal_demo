// Fill out your copyright notice in the Description page of Project Settings.

#include "SpInputPreprocessor.h"
#include "SpGame.h"

#include "GUI/DlgMgr.h"
#include "SprayMgr.h"

#include "Input/Events.h"
#include "Misc/App.h"
#include "InputCoreTypes.h"
#include "Engine/EngineBaseTypes.h"

FSpInputPreprocessor::FSpInputPreprocessor()
{
}

FSpInputPreprocessor::~FSpInputPreprocessor()
{
}

void FSpInputPreprocessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FSpInputPreprocessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (GSpWorld)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape && !InKeyEvent.IsRepeat())
		{
			AGameModeBattle *GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
			if (GameMode)
			{
				if (!UDlgSysSetting::IsInstanced())
				{
					UDlgSysSetting::GetInstance()->Show();
					SwitchInputMode(true, true);
					return true;
				}
			}
		}
		/*else if (InKeyEvent.GetKey() == EKeys::C && !InKeyEvent.IsRepeat())
		{
			AEditLevelGameMode *GameMode = Cast<AEditLevelGameMode>(GSpWorld->GetAuthGameMode());
			if (GameMode)
			{
				ASprayMgr::SaveSprayTexture();
				GameMode->Save();
				return true;
			}
		}*/
	}
	return DlgMgr->PreprocessedInputKey(InKeyEvent.GetKey(), InKeyEvent.IsRepeat() ? IE_Repeat : IE_Pressed);
}

bool FSpInputPreprocessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	return false;
}

bool FSpInputPreprocessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	return false;
}

bool FSpInputPreprocessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	return false;
}
