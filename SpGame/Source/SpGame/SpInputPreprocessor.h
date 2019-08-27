// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/ICursor.h"
#include "Framework/Application/IInputProcessor.h"

enum EInputEvent;
class FSlateApplication;
struct FAnalogInputEvent;
struct FKeyEvent;
struct FPointerEvent;

/**
 * 
 */
class SPGAME_API FSpInputPreprocessor : public IInputProcessor
{
public:
	FSpInputPreprocessor();
	virtual ~FSpInputPreprocessor();

	// IInputProcess overrides
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	//FViewportWorldInteractionManager* WorldInteractionManager;
};
