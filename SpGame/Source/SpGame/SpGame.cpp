// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGame.h"
#include "Modules/ModuleManager.h"
#include "client/dllclient.h"

static bool SpGameExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bHandled = false;

	if (spclient::checkGMCmd(TCHAR_TO_UTF8(Cmd)) == 0) {
		bHandled = true;
	}
	else 
	{
		bHandled = true;
		if (FParse::Command(&Cmd, TEXT("/b"))) 
		{
			TCHAR token[128];
			if (FParse::Token(Cmd, token, ARRAY_COUNT(token), true))
			{
				if (token[0] == '0') {
					spclient::startPVE("/Game/Maps/Test");
				}
				else if (token[0] == '1') {
					spclient::startPVE("/Game/DragonTest");
				}
				else 
				{
					FString path = FString("/Game/Maps/") + token;
					spclient::startPVE(TCHAR_TO_UTF8(*path));
				}
			}
			else {
				spclient::startPVE("/Game/SolidTest");
			}
		}
		else {
			bHandled = false;
		}
	}

	return bHandled;
}

FStaticSelfRegisteringExec SpGameExecRegistration(SpGameExec);

void FSpGameModule::StartupModule()
{
}

void FSpGameModule::ShutdownModule()
{
}

//IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SpGame, "SpGame" );
IMPLEMENT_PRIMARY_GAME_MODULE(FSpGameModule, SpGame, "SpGame");
