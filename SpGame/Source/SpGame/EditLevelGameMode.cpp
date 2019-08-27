// Fill out your copyright notice in the Description page of Project Settings.

#include "EditLevelGameMode.h"
#include "EditLevelPawn.h"
#include "EngineUtils.h"
#include "HighResScreenshot.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/FileManager.h"
#include <m_public.h>

void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}

		delete RegionData->Regions;
		delete RegionData;
			});
	}
}

bool ReplaceMaterial(UStaticMeshComponent *mesh, UMaterial *baseMaterial, UMaterial *newBaseMaterial, TArray<UMaterialInstanceDynamic*> *replacedMaterials)
{
	bool foundSurface = false;
	TArray<class UMaterialInterface *> materials = mesh->GetMaterials();
	for (int i = 0; i < materials.Num(); ++i)
	{
		if (materials[i] /*&& materials[i]->GetBaseMaterial() == baseMaterial*/)
		{
			foundSurface = true;

			UMaterialInstanceDynamic *mat = UMaterialInstanceDynamic::Create(newBaseMaterial ? newBaseMaterial : materials[i], mesh);
			if (mat)
			{
				mesh->SetMaterial(i, mat);
				if (replacedMaterials) {
					replacedMaterials->Add(mat);
				}
			}
		}
	}

	return foundSurface;
}

#undef UpdateResource
void SaveTexture2D(UTexture2D *tex, FString path)
{
	if (nullptr == tex) {
		return;
	}

	tex->UpdateResource();
	FTexture2DMipMap* MM = &tex->PlatformData->Mips[0];

	TArray<FColor> OutBMP;
	int w = MM->SizeX;
	int h = MM->SizeY;

	OutBMP.InsertZeroed(0, w*h);

	FByteBulkData* RawImageData = &MM->BulkData;

	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	for (int i = 0; i < (w*h); ++i)
	{
		OutBMP[i] = FormatedImageData[i];
		OutBMP[i].A = 255;
	}

	RawImageData->Unlock();
	FIntPoint DestSize(w, h);

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	HighResScreenshotConfig.SaveImage(path, OutBMP, DestSize, &ResultPath);
}

void SaveRenderTarget2D(UTextureRenderTarget2D *rt, FString path)
{
	FTextureRenderTargetResource *rtResource = rt->GameThread_GetRenderTargetResource();
	int w = rt->GetSurfaceWidth(), h = rt->GetSurfaceHeight();
	EPixelFormat fmt = rt->GetFormat();

	FIntPoint destSize(w, h);
	FString resultPath;
	FHighResScreenshotConfig& ssConfig = GetHighResScreenshotConfig();


	if (fmt == PF_FloatRGBA)
	{
		TArray<FFloat16Color> outBMP;
		rtResource->ReadFloat16Pixels(outBMP);
		for (FFloat16Color &c : outBMP) {
			c.A = 1.0f;
		}

		ssConfig.bCaptureHDR = true;
		ssConfig.SaveImage(path, outBMP, destSize, &resultPath);
	}
	else
	{
		TArray<FColor> outBMP;
		rtResource->ReadPixels(outBMP);
		for (FColor &c : outBMP) {
			c.A = 255;
		}

		ssConfig.bCaptureHDR = false;
		ssConfig.SaveImage(path, outBMP, destSize, &resultPath);
	}
}


AEditLevelGameMode::AEditLevelGameMode(const FObjectInitializer &objInitor)
	: Super(objInitor)
{
	DefaultPawnClass = AEditLevelPawn::StaticClass();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		FName name = MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), TEXT("AEditLevelGameMode::RtHDR"));
		RtHDR = NewObject<UTextureRenderTarget2D>(this, name);
		RtHDR->InitCustomFormat(2048, 2048, PF_FloatRGBA, false);
		RtHDR->ClearColor = FLinearColor::Black;

		name = MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), TEXT("AEditLevelGameMode::RtLDR"));
		RtLDR = NewObject<UTextureRenderTarget2D>(this, name);
		RtLDR->InitCustomFormat(2048, 2048, PF_B8G8R8A8, false);
		RtLDR->ClearColor = FLinearColor::Black;
	}

	CurrSprayMeshMaterial = nullptr;
}


void AEditLevelGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld *world = GetWorld();
	if (nullptr == world) {
		return;
	}

	DefaultSprayMaterial = LoadObject<UMaterial>(world, TEXT("/Game/SprayMeshMaterial.SprayMeshMaterial"));
	WorldPosMaterial = LoadObject<UMaterial>(world, TEXT("/Game/WorldPosMaterial.WorldPosMaterial"));
	WorldNormalMaterial = LoadObject<UMaterial>(world, TEXT("/Game/WorldNormalMaterial.WorldNormalMaterial"));
	WorldPosMaterial->bUsedWithStaticLighting = true;
	CurrSprayMeshMaterial = WorldPosMaterial;

	for (TActorIterator<AActor> it(world); it; ++it)
	{
		ASpStaticMeshActor *actor = Cast<ASpStaticMeshActor>(*it);
		if (actor && actor->bEnableSpray)
		{
			TArray<UActorComponent *> comps = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
			for (int i = 0; i < comps.Num(); ++i)
			{
				UStaticMeshComponent *comp = Cast<UStaticMeshComponent>(comps[i]);
				TArray<UMaterialInstanceDynamic*> mats;
				if (ReplaceMaterial(comp, DefaultSprayMaterial, WorldPosMaterial, &mats))
				{
					if (mats.Num() > 1) {
						UE_LOG(LogTemp, Error, TEXT("%s can't contain multiple UStaticMeshComponent or contain multiple material slots."), *actor->GetName());
					}

					FSprayMesh mesh;
					mesh.actor = actor;
					mesh.comp = comp;
					mesh.mat = mats[0];
					SprayMeshs.Add(mesh);
				}
			}
		}		
	}
}

static void unit_bounds(FVector &cent, FVector &size, const FVector &tar_cent, const FVector &tar_size)
{
	FVector my_min = cent - size, my_max = cent + size;
	my_min.ComponentMin(tar_cent - tar_size);
	my_max.ComponentMax(tar_cent + tar_size);

	size = (my_max - my_min) * 0.5f;
	cent = my_min + size;
}

FString getLevelDataPhysPath(UWorld *world)
{
	FString lvlPath = world->GetOutermost()->FileName.ToString();
	FString lvl_abs_path = FPaths::GameContentDir();
	if (lvlPath.Find(TEXT("/Game/")) == 0) {
		lvl_abs_path += &lvlPath[6];
	}
	else {
		lvl_abs_path += &lvlPath[1];
	}
	lvl_abs_path += TEXT("/");
	return lvl_abs_path;
}

FString getLevelDataEnginePath(UWorld *world)
{
	FString lvl_abs_path = world->GetOutermost()->FileName.ToString();
	lvl_abs_path += TEXT("/");
	return lvl_abs_path;
}

static void SerializeStaticMeshComponentGeom(FArchive *ar, UStaticMeshComponent *comp)
{
	const FTransform &world = comp->GetComponentTransform();

	const FStaticMeshLODResources &lod = comp->GetStaticMesh()->GetLODForExport(0);
	const FIndexArrayView &ia = lod.IndexBuffer.GetArrayView();
	Uint32 index_count = lod.GetNumTriangles() * 3;
	Uint32 vertex_count = lod.PositionVertexBuffer.GetNumVertices();

	Uint16 *ia16 = new Uint16[index_count];
	for (unsigned j = 0; j < index_count/3; ++j)	//It's strange, triangle indices are Right handed...
	{
		ia16[j * 3 + 0] = ia[j * 3 + 0];
		ia16[j * 3 + 1] = ia[j * 3 + 2];
		ia16[j * 3 + 2] = ia[j * 3 + 1];		
	}

	FVector *pos = new FVector[vertex_count], *norm = new FVector[vertex_count];
	for (unsigned j = 0; j < vertex_count; ++j) {
		pos[j] = world.TransformPosition(lod.PositionVertexBuffer.VertexPosition(j));
		norm[j] = world.TransformVectorNoScale(lod.VertexBuffer.VertexTangentZ(j));
	}

	ar->Serialize(&index_count, 4);
	ar->Serialize(&vertex_count, 4);
	ar->Serialize(ia16, index_count * 2);
	ar->Serialize(pos, vertex_count * 12);
	ar->Serialize(norm, vertex_count * 12);

	SAFE_DELETE_ARRAY(ia16);
	SAFE_DELETE_ARRAY(pos);
	SAFE_DELETE_ARRAY(norm);
}

extern int CalcSprayTextureSize(float size);
void AEditLevelGameMode::Save()
{
	APlayerController *pc = GetGameInstance()->GetFirstLocalPlayerController();
	if (nullptr == pc) {
		return;
	}

	AEditLevelPawn *ch = Cast<AEditLevelPawn>(pc->GetPawn());
	if (nullptr == ch) {
		return;
	}

	if (SprayMeshs.Num() == 0) {
		return;
	}

	FString lvl_abs_path = getLevelDataPhysPath(SprayMeshs[0].actor->GetWorld());

	// Render and save WorldPos (HDR), WorldNormal (LDR).
	for (int k = 0; k < 2; ++k)
	{
		for (int i = 0; i < SprayMeshs.Num(); ++i)
		{
			FSprayMesh &mesh = SprayMeshs[i];
			FBox bbox = mesh.actor->GetComponentsBoundingBox(); 
			int size = MIN(1024, CalcSprayTextureSize(bbox.GetExtent().Size()));

			UTextureRenderTarget2D *rt = RtHDR;
			if (k == 0 || 1)	
			{
				RtHDR->InitCustomFormat(size, size, PF_FloatRGBA, false);

				ch->SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
				ch->SceneCapture->TextureTarget = RtHDR;
			}
			else 
			{
				RtLDR->InitCustomFormat(size, size, PF_B8G8R8A8, false);

				ch->SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
				ch->SceneCapture->TextureTarget = RtLDR;
				rt = RtLDR;
			}

			int w = rt->GetSurfaceWidth(), h = rt->GetSurfaceHeight();
			EPixelFormat fmt = rt->GetFormat();

			// render worldpos/normal for each static-mesh actor.
			ch->SceneCapture->ShowOnlyComponents.Empty();
			ch->SceneCapture->ShowOnlyComponent(mesh.comp);

			// place camera to look-at the mesh actor.
			FVector cameraLoc = bbox.GetCenter() - FVector(1, 0, 0) * bbox.GetExtent().Size() * 2.0f;
			FRotator cameraRot(0, 0, 0);
			ch->SetActorLocationAndRotation(cameraLoc, cameraRot);

			// replace the mesh actor's material;			
			if (k == 0 && CurrSprayMeshMaterial != WorldPosMaterial) {
				ReplaceMaterial(mesh.comp, CurrSprayMeshMaterial, WorldPosMaterial, nullptr);
			}
			else if (k == 1 && CurrSprayMeshMaterial != WorldNormalMaterial) {
				ReplaceMaterial(mesh.comp, CurrSprayMeshMaterial, WorldNormalMaterial, nullptr);
			}

			TArray<class UMaterialInterface *> materials = mesh.comp->GetMaterials();
			for (UMaterialInterface *mat : materials)
			{
				UMaterialInstanceDynamic *matd = Cast<UMaterialInstanceDynamic>(mat);
				if (matd) {
					matd->SetVectorParameterValue(TEXT("BBoxMin"), bbox.Min);
				}
			}

			// render to TextureRenderTarget.
			ch->SceneCapture->CaptureScene();
			FlushRenderingCommands();

			// save to disk
			FString path = lvl_abs_path + mesh.actor->GetFName().ToString();
			if (k == 0) {
				path += TEXT("_WorldPos");
			}
			else {
				path += TEXT("_WorldNormal");
			}

			FIntPoint destSize(w, h);
			FString resultPath;
			FTextureRenderTargetResource *rtResource = rt->GameThread_GetRenderTargetResource();
			FHighResScreenshotConfig& ssConfig = GetHighResScreenshotConfig();

			if (fmt == PF_FloatRGBA)
			{
				TArray<FFloat16Color> outBMP;
				rtResource->ReadFloat16Pixels(outBMP);
				for (FFloat16Color &c : outBMP) {
					c.A = 1.0f;
				}

				ssConfig.bCaptureHDR = true;
				ssConfig.SaveImage(path, outBMP, destSize, &resultPath);
			}
			else if (fmt == PF_B8G8R8A8)
			{
				TArray<FColor> outBMP;
				rtResource->ReadPixels(outBMP);
				for (FColor &c : outBMP) {
					c.A = 255;
				}

				ssConfig.bCaptureHDR = false;
				ssConfig.SaveImage(path, outBMP, destSize, &resultPath);
			}
			
		}

		CurrSprayMeshMaterial = k == 0 ? WorldPosMaterial : WorldNormalMaterial;
	}	

	// Save collision geometry data
	// Generate geometry file path name;
	FString path = lvl_abs_path + TEXT("Geometry.bin");

	// Create geometry file;
	FArchive *ar = IFileManager::Get().CreateFileWriter(*path);
	if (ar != nullptr)
	{
		// Calc lvl spray bounding box; Collect all geometry with collision;
		TArray<UStaticMeshComponent*> none_spray_meshs;
		FVector lvl_center, lvl_size;
		bool lvl_box_init = false;

		for (TActorIterator<AActor> it(GetWorld()); it; ++it)
		{
			AStaticMeshActor *actor = Cast<AStaticMeshActor>(*it);
			if (actor)
			{
				ASpStaticMeshActor *spray_mesh = Cast<ASpStaticMeshActor>(actor);

				bool is_collider = false;
				bool is_spraymesh = spray_mesh && spray_mesh->bEnableSpray;

				TArray<UActorComponent *> comps = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
				for (int i = 0; i < comps.Num(); ++i)
				{
					UStaticMeshComponent *comp = Cast<UStaticMeshComponent>(comps[i]);
					if (comp)
					{
						if (comp->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
						{
							is_spraymesh = true;

							if (!is_spraymesh) {
								none_spray_meshs.Add(comp);
							}
						}
					}
				}

				FVector actor_center, actor_size;
				actor->GetActorBounds(true, actor_center, actor_size);
				if (lvl_box_init) {
					unit_bounds(lvl_center, lvl_size, actor_center, actor_size);
				}
				else
				{
					lvl_box_init = true;

					lvl_center = actor_center;
					lvl_size = actor_size;					
				}
			}
		}

		Uint32 spray_mesh_count = SprayMeshs.Num();
		Uint32 none_spray_count = none_spray_meshs.Num();

		// Save to geometry file;
		ar->Serialize(&spray_mesh_count, 4);
		ar->Serialize(&none_spray_count, 4);
		ar->Serialize(&lvl_center, 12);
		ar->Serialize(&lvl_size, 12);

		for (int i = 0; i < SprayMeshs.Num(); ++i)
		{
			FSprayMesh &sm = SprayMeshs[i];
			const char *actor_name = TCHAR_TO_UTF8(*sm.actor->GetFName().ToString());
			Uint16 name_len = strlen(actor_name);

			ar->Serialize(&name_len, 2);
			ar->Serialize((void*)actor_name, name_len);
			int align_len = 4 - ((2 + name_len) & 3);
			if (align_len) {
				Uint32 t = 0;
				ar->Serialize(&t, align_len);
			}

			SerializeStaticMeshComponentGeom(ar, sm.comp);
		}

		for (unsigned i = 0; i < none_spray_count; ++i)	{
			SerializeStaticMeshComponentGeom(ar, none_spray_meshs[i]);
		}

		// Close geometry file;
		ar->Close();
		ar = nullptr;
	}	

	// Save nav-mesh
	/*
	const UNavigationSystem *nav_system = GetWorld()->GetNavigationSystem();
	for (int i = 0; i < nav_system->NavDataSet.Num(); ++i)
	{
		const ARecastNavMesh *nav_data = Cast<const ARecastNavMesh>(nav_system->NavDataSet[i]);
		if (nav_data)
		{

		}
	}
	*/
}
