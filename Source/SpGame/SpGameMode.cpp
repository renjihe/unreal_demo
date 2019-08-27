// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameMode.h"
#include "SpGame.h"
#include "GUI/DlgBase.h"
#include "GUI/DlgMgr.h"

#include "client/dllclient.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "GameFramework/GameUserSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

/*void OnSmLoginSuccess(const char *account)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString(UTF8_TO_TCHAR(account)));
}*/

UWorldProxy GSpWorld;
void SetCurrSpWorld(UWorld *world)
{
	GSpWorld = world;
	UDlgBase::SetCurrWorld(world);

	if (nullptr == world) {
		DlgMgr->DestroyAllDlg();
	}
}

void SwitchInputMode(bool bGuiMode, bool bGuiOnly)
{
	if (GSpWorld)
	{
		APlayerController *controller = GSpWorld->GetFirstPlayerController();
		if (controller)
		{
			if (bGuiMode)
			{
				if (bGuiOnly) {
					UWidgetBlueprintLibrary::SetInputMode_UIOnly(controller);
				}
				else {
					UWidgetBlueprintLibrary::SetInputMode_GameAndUI(controller, nullptr, true, false);
				}
				controller->bShowMouseCursor = true;
				controller->bEnableClickEvents = true;
				controller->bEnableMouseOverEvents = true;
			}
			else
			{
				controller->bShowMouseCursor = false;
				controller->bEnableClickEvents = false;
				controller->bEnableMouseOverEvents = false;
				UWidgetBlueprintLibrary::SetInputMode_GameOnly(controller);
			}
		}
	}
}


ASpGameMode::ASpGameMode(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	//PrimaryComponentTick.bTickEvenWhenPaused = true;
	//SetTickGroup(TG_PostUpdateWork);
	bInited = false;
}

void ASpGameMode::BeginPlay()
{
	Super::BeginPlay();
	//GEngine->GameUserSettings->SetScreenResolution(FIntPoint(1680, 1080));
	//GEngine->GameUserSettings->ApplyResolutionSettings(false);
}

void ASpGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInited)
	{
		OnInited();
		bInited = true;
	}
}

void ASpGameMode::OnInited()
{
	if (nullptr == GSpWorld) {
		SetCurrSpWorld(GetWorld());
	}

	SwitchInputMode(true);
}

void ASpGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::EndPlayInEditor) {
		spclient::logout();
	}

	Super::EndPlay(EndPlayReason);

	if (nullptr != GSpWorld) {
		SetCurrSpWorld(nullptr);
	}
}

bool CalcWorldPointUV(UStaticMeshComponent *compo, int faceIndex, FVector worldLocation, FVector2D &outUV)
{
	UStaticMesh *mesh = compo->GetStaticMesh();
	if (mesh)
	{
		const FStaticMeshLODResources &lod = mesh->GetLODForExport(0);
		int triangles = lod.GetNumTriangles();

		const FIndexArrayView &ia = lod.IndexBuffer.GetArrayView();
		if (faceIndex * 3 + 2 < ia.Num())
		{
			const uint8 *data = lod.VertexBuffer.GetRawVertexData();
			FVector2D tc[3];
			FVector p[4];

			for (int i = 0; i < 3; ++i)
			{
				int index = lod.IndexBuffer.GetArrayView()[faceIndex*3 + i];
				p[i+1] = lod.PositionVertexBuffer.VertexPosition(index);
				tc[i] = lod.VertexBuffer.GetVertexUV(index, 1);
			}

			// calculate uv.
			FTransform trans = compo->GetComponentTransform();
			p[0] = trans.InverseTransformPosition(worldLocation);

			//P.x = (1 - u - v) * P1.x + u * P2.x + v * P3.x
			//P.y = (1 - u - v) * P1.y + u * P2.y + v * P3.y
			float u = 1.0f, v = 1.0f;
			float u1 = (p[2].X - p[1].X) * (p[3].Y - p[1].Y) - (p[2].Y - p[1].Y) * (p[3].X - p[1].X);
			if (u1 > KINDA_SMALL_NUMBER || u1 < -KINDA_SMALL_NUMBER) {
				u = ((p[0].X - p[1].X) * (p[3].Y - p[1].Y) - (p[0].Y - p[1].Y) * (p[3].X - p[1].X)) / u1;
			}
			else 
			{
				u1 = (p[2].X - p[1].X) * (p[3].Z - p[1].Z) - (p[2].Z - p[1].Z) * (p[3].X - p[1].X);
				if (u1 > KINDA_SMALL_NUMBER || u1 < -KINDA_SMALL_NUMBER) {
					u = ((p[0].X - p[1].X) * (p[3].Z - p[1].Z) - (p[0].Z - p[1].Z) * (p[3].X - p[1].X)) / u1;
				}
				else
				{
					u1 = (p[2].Y - p[1].Y) * (p[3].Z - p[1].Z) - (p[2].Z - p[1].Z) * (p[3].Y - p[1].Y);
					if (u1 > KINDA_SMALL_NUMBER || u1 < -KINDA_SMALL_NUMBER) {
						u = ((p[0].Y - p[1].Y) * (p[3].Z - p[1].Z) - (p[0].Z - p[1].Z) * (p[3].Y - p[1].Y)) / u1;
					}
				}
			}

			float v1 = (p[3].X - p[1].X);
			if (v1 > KINDA_SMALL_NUMBER || v1 < -KINDA_SMALL_NUMBER) {
				v = ((p[0].X - p[1].X) - (p[2].X - p[1].X) * u) / v1;
			}
			else 
			{
				v1 = (p[3].Y - p[1].Y);
				if (v1 > KINDA_SMALL_NUMBER || v1 < -KINDA_SMALL_NUMBER) {
					v = ((p[0].Y - p[1].Y) - (p[2].Y - p[1].Y) * u) / v1;
				}
				else
				{
					v1 = (p[3].Z - p[1].Z);
					if (v1 > KINDA_SMALL_NUMBER || v1 < -KINDA_SMALL_NUMBER) {
						v = ((p[0].Z - p[1].Z) - (p[2].Z - p[1].Z) * u) / v1;
					}
				}
			}

			outUV = tc[0] * (1.0f - u - v) + tc[1] * u + tc[2] * v;
			return true;
		}
	}

	return false;
}

void ASpGameMode::Test(UObject* WorldContextObject)
{
	FVector mouseLoc, mouseDir;
	APlayerController *pc = WorldContextObject->GetWorld()->GetFirstPlayerController();
	if (pc && pc->DeprojectMousePositionToWorld(mouseLoc, mouseDir))
	{
		TArray<AActor*> ignore;
		FHitResult result;
		if (UKismetSystemLibrary::LineTraceSingle(WorldContextObject, mouseLoc, mouseLoc + mouseDir * 1000, (ETraceTypeQuery)ECC_WorldStatic
			, true, ignore, EDrawDebugTrace::ForDuration, result, true))
		{
			UStaticMeshComponent *compo = Cast<UStaticMeshComponent>(result.GetComponent());
			if (compo)
			{
				FVector2D uv;
				if (CalcWorldPointUV(compo, result.FaceIndex, result.Location, uv))
				{

				}				
			}
		}
	}
}