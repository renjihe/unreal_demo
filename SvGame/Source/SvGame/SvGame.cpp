// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "Engine/World.h"
#include "PlayerControllerSV.h"
#include "CoreMisc.h"

static bool SvGameExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bHandled = false;

	if (InWorld)
	{
		APlayerControllerSV* Controller = Cast<APlayerControllerSV>(InWorld->GetFirstPlayerController());
		if (Controller)
		{
			if (FParse::Command(&Cmd, TEXT("SeamLess")))
			{
				bHandled = true;
				Controller->ServerTestCmd("SeamLess", Cmd);
			}
		}
	}

	return bHandled;
}

FStaticSelfRegisteringExec SvGameExecRegistration(SvGameExec);

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SvGame, "SvGame" );
