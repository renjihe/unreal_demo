// Fill out your copyright notice in the Description page of Project Settings.

#include "EditLevelPawn.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

AEditLevelPawn::AEditLevelPawn(const FObjectInitializer &objInitor)
	: Super(objInitor)
{
	//static ConstructorHelpers::FObjectFinder 

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>("SceneCapture");
	SceneCapture->SetupAttachment(RootComponent);
	
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->ShowFlags.SetAmbientCubemap(false);
		SceneCapture->ShowFlags.SetAntiAliasing(false);
		SceneCapture->ShowFlags.SetAtmosphericFog(false);
		SceneCapture->ShowFlags.SetSkyLighting(false);
		SceneCapture->ShowFlags.SetPostProcessing(false);
		SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
	}	
}

