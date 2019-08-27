// Fill out your copyright notice in the Description page of Project Settings.

#include "DlgBattleResult.h"
#include "Internationalization/Text.h"
#include "DlgSysSetting.h"
#include "Character/SpCharacter.h"
#include "Character/EntityComponent.h"
#include "Res/LookRes.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"

#include "client/dllclient.h"

IMPLEMENT_DLGBASE_ISNTANCE(UDlgBattleResult)

void UDlgBattleResult::Create()
{
	UDlgBase::Create("BattleResult", false);
}

void UDlgBattleResult::Init()
{
	Formate.MinimumIntegralDigits = 1;
	Formate.MaximumIntegralDigits = 3;
	Formate.MinimumFractionalDigits = 1;
	Formate.MaximumFractionalDigits = 1;

	if (Widget)
	{
		ImageCharacter = (UImage *)GetChild("ImageCharacter");
		if (ImageCharacter)
		{
			UMaterialInterface *mat = Cast<UMaterialInterface>(ImageCharacter->Brush.GetResourceObject());
			if (mat)
			{
				UMaterialInstanceDynamic *dymat = UMaterialInstanceDynamic::Create(mat, GetWorld());
				CharacterRenderTarget = NewObject<UTextureRenderTarget2D>(GetWorld(), UTextureRenderTarget2D::StaticClass());
				CharacterRenderTarget->InitAutoFormat(1536.f, 512.f);
				dymat->SetTextureParameterValue(TEXT("Texture"), CharacterRenderTarget);
				ImageCharacter->Brush.SetResourceObject(dymat);
			}
		}

		MapResult = (UImage *)GetChild("MapResult");
		if (MapResult)
		{
			UMaterialInterface *mat = Cast<UMaterialInterface>(MapResult->Brush.GetResourceObject());
			if (mat)
			{
				UMaterialInstanceDynamic *dymat = UMaterialInstanceDynamic::Create(mat, GetWorld());
				MapRenderTarget = NewObject<UTextureRenderTarget2D>(GetWorld(), UTextureRenderTarget2D::StaticClass());
				MapRenderTarget->InitAutoFormat(512.f, 512.f);
				dymat->SetTextureParameterValue(TEXT("Texture"), MapRenderTarget);
				MapResult->Brush.SetResourceObject(dymat);
			}
		}

		TextAtkerPercent = (UTextBlock *)GetChild("AtkerPercent");
		TextDeferPercent = (UTextBlock *)GetChild("DeferPercent");
		TextAtkerResult = (UTextBlock *)GetChild("AtkerResult");
		TextDeferResult = (UTextBlock *)GetChild("DeferResult");
		TextAtkerExp = (UTextBlock *)GetChild("AtkerExp");
		TextAtkerExp = (UTextBlock *)GetChild("AtkerExp");

		ProgressBar = (UImage *)GetChild("ProgressBar");
		if (ProgressBar)
		{
			UMaterialInterface *mat = Cast<UMaterialInterface>(ProgressBar->Brush.GetResourceObject());
			if (mat)
			{
				ProgressMat = UMaterialInstanceDynamic::Create(mat, GetWorld());
				ProgressBar->Brush.SetResourceObject(ProgressMat);
			}
		}

		BtnSysSetting = (UButton *)GetChild("BtnGoOn");
		if (BtnSysSetting)
		{
			BtnSysSetting->OnClicked.AddDynamic(this, &UDlgBattleResult::SysSettingClicked);
			//BtnSysSetting->SetVisibility(ESlateVisibility::Hidden);
		}

		RenderCharacter();
	}
}

float UDlgBattleResult::CalcRange(float total, float half, float time)
{
	float y;
	if (time < T1)	{
		y = time * half / T1;
	} 
	else if(time < T2) {
		y = half;
	}
	else if (time < T3)	{
		y = half + (time - T2) * (total - half) / (T3 - T2);
	}
	else
	{
		y = total;
		ShowResult();
	}
	return y;
}

extern void LoadMap(const char *path);
void UDlgBattleResult::UpdateFrame(float DeltaSeconds)
{
	Time += DeltaSeconds;

	if (TextAtkerPercent) {
		TextAtkerPercent->SetText(FText::AsPercent(CalcRange(AtkerScore, HalfofScore, Time), &Formate));
	}
	if (TextDeferPercent) {
		TextDeferPercent->SetText(FText::AsPercent(CalcRange(DeferScore, HalfofScore, Time), &Formate));
	}
	if (ProgressBar)
	{
		ProgressMat->SetScalarParameterValue(TEXT("AtkerScore"), CalcRange(AtkerPercent, HalfofPercent, Time));
		ProgressMat->SetScalarParameterValue(TEXT("DeferScore"), CalcRange(DeferPercent, HalfofPercent, Time));
	}
}

void UDlgBattleResult::UpdateScore(float atkerScore, float deferScore)
{
	AtkerScore = atkerScore;
	DeferScore = deferScore;

	if ((atkerScore + deferScore) == 0 ) {
		AtkerPercent = DeferPercent = 0;
	}
	else
	{
		AtkerPercent = atkerScore / (atkerScore + deferScore);
		DeferPercent = 1 - AtkerPercent;
	}

	HalfofPercent = ( AtkerPercent > DeferPercent ? DeferPercent : AtkerPercent ) * 0.5;
	HalfofScore = ( AtkerScore > DeferScore ? DeferScore : AtkerScore ) * 0.5;

	Time = 0;
}

void UDlgBattleResult::SysSettingClicked()
{
	LoadMap("/Game/Maps/Campsite");
}

int32 UDlgBattleResult::GetWinner()
{
	return AtkerScore > DeferScore ? 0 : 1;
}

void UDlgBattleResult::RenderCharacter()
{
	FVector camPos = FVector(0, 0, 5000);
	FRotator camRot = FVector(0, 0, -1).Rotation();
	
	if (GetWorld())
	{
		ASceneCapture2D *sceneCapture = GetWorld()->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(), camPos, camRot);
		sceneCapture->GetCaptureComponent2D()->TextureTarget = MapRenderTarget;

		TArray<AActor *> actors;

		const int32 PlayerNumber = GetWorld()->GetNumPawns();
		const float Length = CalcCameraPos(PlayerNumber);
		const float offset = Length / (PlayerNumber / 2 + 1);

		FVector winPos  = camPos + sceneCapture->GetActorForwardVector() * CalcCameraPos(PlayerNumber) + sceneCapture->GetActorRightVector();
		FVector failPos = camPos + sceneCapture->GetActorForwardVector() * CalcCameraPos(PlayerNumber) + sceneCapture->GetActorRightVector()*(-1);
		
		for (TActorIterator<ASpCharacter> It(GetWorld()); It; ++It)
		{
			ASpCharacter *character = *It;
			if (character)
			{
				actors.Add(character);

				FRotator rot = FRotationMatrix::MakeFromXZ(sceneCapture->GetActorForwardVector()*(-1),sceneCapture->GetActorUpVector()).Rotator();

				if (character->Faction == GetWinner())
				{
					character->SetActorRotation(rot);
					winPos += sceneCapture->GetActorRightVector() * (-1) * offset;
					character->SetActorLocation(winPos);

					character->GetEntityComponent()->PlayAction(AnimationType::Win);
				}
				else
				{
					character->SetActorRotation(rot);
					failPos += sceneCapture->GetActorRightVector() * offset;
					character->SetActorLocation(failPos);

					character->GetEntityComponent()->PlayAction(AnimationType::Lose);					
				}

				character->EntityComponent->Show(false);
				TArray<AActor *> compontents;
				character->GetAttachedActors(compontents);
				actors.Append(compontents);
			}
		}

		sceneCapture->GetCaptureComponent2D()->CaptureScene();

		for (TActorIterator<ASpCharacter> It(GetWorld()); It; ++It)
		{
			ASpCharacter *character = *It;
			if (character && character->EntityComponent.IsValid()) {
				character->EntityComponent->Show(true);
			}
		}

		sceneCapture->GetCaptureComponent2D()->TextureTarget = CharacterRenderTarget;
		sceneCapture->GetCaptureComponent2D()->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		sceneCapture->GetCaptureComponent2D()->ShowFlags.SetDirectLighting(false);

		sceneCapture->GetCaptureComponent2D()->ShowOnlyActors.Append(actors);
	}
}

float UDlgBattleResult::CalcCameraPos(int32 playerNumber)
{
	return 5400 / (20 - playerNumber);
}

void UDlgBattleResult::ShowResult()
{
	if (TextAtkerResult) {
		TextAtkerResult->SetText(GetWinner() ? FText::FromString(TEXT("Lose")) : FText::FromString(TEXT("Win")));
	}
	if (TextDeferResult) {
		TextDeferResult->SetText(GetWinner() ? FText::FromString(TEXT("Win")) : FText::FromString(TEXT("Lose")));
	}
}