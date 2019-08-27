// Fill out your copyright notice in the Description page of Project Settings.

#include "SpGameEd.h"
#include "Modules/ModuleManager.h"
//#include "BaseEditorTool.h"
#include "LevelEditor.h"
#include "EditorStyleSet.h"
#include "SpStaticMeshActor.h"
#include "SprayMaskFactory.h"
#include "BinAssetFactory.h"
#include "client/dllclient.h"

#define LOCTEXT_NAMESPACE "FSpGameEdModule"

void FSpGameEditorCommands::RegisterCommands()
{
	UI_COMMAND(Command, "Export Level", "Export level tooltip.", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(Command_reserved, "Reversed", "", EUserInterfaceActionType::Button, FInputGesture());
}

void ExportLevel();
void OnMenuExportLevel()
{
	ExportLevel();
}

void OnMenu_reserved()
{
	
}

void FSpGameEdModule::StartupModule()
{
	FSpGameEditorCommands::Register();

	EditorCommands = MakeShareable(new FUICommandList);
	EditorCommands->MapAction(FSpGameEditorCommands::Get().Command, FExecuteAction::CreateStatic(&OnMenuExportLevel));
	EditorCommands->MapAction(FSpGameEditorCommands::Get().Command_reserved, FExecuteAction::CreateStatic(&OnMenu_reserved));

	//EditorCommands->MapAction(FSpGameEditorCommands::Get().SubmenuTestCommand01
	//		, FExecuteAction::CreateLambda([]() {
	//			UE_LOG(LogTemp, Warning, TEXT("SUBMENU 01 works!!!")); 
	//		}) 
	//);

	MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("FileLoadAndSave", EExtensionHook::After, EditorCommands, FMenuExtensionDelegate::CreateStatic(&FSpGameEdModule::AddMenuEntry));

	FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");	
	levelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// Register my asset;
	BinAssetTypeActions = MakeShareable(new FAssetTypeActions_BinAsset);
	SprayMaskTypeActions = MakeShareable(new FAssetTypeActions_SprayMask);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(BinAssetTypeActions.ToSharedRef());
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SprayMaskTypeActions.ToSharedRef());

}

void FSpGameEdModule::ShutdownModule()
{
	if (MenuExtender.IsValid() && FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		levelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
	}
}

void FSpGameEdModule::AddMenuEntry(class FMenuBuilder &menuBuilder)
{
	menuBuilder.BeginSection("SpEditorMenu", TAttribute<FText>(FText::FromString("SpGame Utilities")));
	menuBuilder.AddMenuEntry(FSpGameEditorCommands::Get().Command);
	menuBuilder.AddMenuEntry(FSpGameEditorCommands::Get().Command_reserved);
	//menuBuilder.AddSubMenu(FText::FromString("list"), FText::FromString("Tooltip list"), FNewMenuDelegate::CreateRaw(this, &FSpGameEdModule::FillSubmenuList));
	menuBuilder.EndSection();

	//TAttribute<FText> m1_name(FText::FromString("SpGame Export Level")), m1_tooltip(FText::FromString("Tooltip: Export level geometry data."));
	//FSlateIcon m1_icon;// (FEditorStyle::GetStyleSetName(), "Icons.IconLarge", "Icons.IconSmall");
	//FUIAction m1_act(FExecuteAction::CreateStatic(&OnMenuExportLevel), EUIActionRepeatMode::RepeatEnabled);

	//menuBuilder.AddMenuEntry(m1_name, m1_tooltip, m1_icon, m1_act, NAME_None, EUserInterfaceActionType::Button, NAME_None);
}

//IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SpGame, "SpGame" );
IMPLEMENT_GAME_MODULE(FSpGameEdModule, "SpGameEd");

#undef LOCTEXT_NAMESPACE

//--------------------------------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------------------------------

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SceneCapture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneCaptureComponent2D.h"
#include "EngineUtils.h"
#include "HighResScreenshot.h"
#include "SpStaticMeshActor.h"
#include <m_public.h>

static void createRenderTarget(UWorld *world, UTextureRenderTarget2D *&rtHDR, UTextureRenderTarget2D *&rtLDR)
{
	rtHDR = NewObject<UTextureRenderTarget2D>();
	rtHDR->InitCustomFormat(2048, 2048, PF_FloatRGBA, false);
	rtHDR->ClearColor = FLinearColor::Black;

	rtLDR = NewObject<UTextureRenderTarget2D>();
	rtLDR->InitCustomFormat(2048, 2048, PF_B8G8R8A8, false);
	rtLDR->ClearColor = FLinearColor::Black;
}


static bool replaceMaterial(UStaticMeshComponent *mesh, UMaterial *baseMaterial, UMaterial *newBaseMaterial, TArray<UMaterialInstanceDynamic*> *replacedMaterials, TArray<UMaterialInterface*> *originMaterials = nullptr)
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

				if (originMaterials) {
					originMaterials->Add(materials[i]);
				}
			}
		}
	}

	return foundSurface;
}

static FString getLevelDataPhysPath(UWorld *world)
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

struct FSprayMesh
{
	ASpStaticMeshActor *actor;

	UStaticMeshComponent *comp;

	UMaterialInstanceDynamic *mat;

	UMaterialInterface *origin_mat;
};


static void collectSprayMeshs(UWorld *world, TArray<FSprayMesh> &sprayMeshs, UMaterial *&defaultSprayMat, UMaterial *&worldPosMat, UMaterial *&worldNormalMat)
{
	if (nullptr == world) {
		return;
	}

	defaultSprayMat = LoadObject<UMaterial>(world, TEXT("/Game/SprayMeshMaterial.SprayMeshMaterial"));
	worldPosMat = LoadObject<UMaterial>(world, TEXT("/Game/WorldPosMaterial.WorldPosMaterial"));
	worldPosMat->bUsedWithStaticLighting = true;
	worldNormalMat = LoadObject<UMaterial>(world, TEXT("/Game/WorldNormalMaterial.WorldNormalMaterial"));

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
				TArray<UMaterialInterface*> origin_mats;
				if (replaceMaterial(comp, defaultSprayMat, worldPosMat, &mats, &origin_mats))
				{
					if (mats.Num() > 1) {
						UE_LOG(LogTemp, Error, TEXT("%s can't contain multiple UStaticMeshComponent or contain multiple material slots."), *actor->GetName());
					}

					FSprayMesh mesh;
					mesh.actor = actor;
					mesh.comp = comp;
					mesh.mat = mats[0];
					mesh.origin_mat = origin_mats[0];
					sprayMeshs.Add(mesh);
				}
			}
		}
	}
}

static void serialize_align(FArchive *ar, unsigned serialized)
{
	int align_len = 4 - (serialized & 3);
	if (align_len && align_len != 4) {
		Uint32 t = 0;
		ar->Serialize(&t, align_len);
	}
}

static void serializeStaticMeshComponentGeom(FArchive *ar, UStaticMeshComponent *comp, bool bEnableSubdMesh = false)
{
	UStaticMesh *mesh = comp->GetStaticMesh();
	if (nullptr == mesh)
	{
		int count = 0;
		ar->Serialize(&count, 4);
		ar->Serialize(&count, 4);
		return;
	}

	if (bEnableSubdMesh)
	{
		FString path = mesh->GetOutermost()->FileName.ToString();
		FString name = FPaths::GetCleanFilename(path);
		path = FPaths::GetPath(path) + "/Spray/" + name + "." + name;

		UStaticMesh *spray_mesh = LoadObject<UStaticMesh>(nullptr, *path);
		if (spray_mesh) {
			mesh = spray_mesh;
		}
	}

	const FTransform &world = comp->GetComponentTransform();

	const FStaticMeshLODResources &lod = mesh->GetLODForExport(0);
	const FIndexArrayView &ia = lod.IndexBuffer.GetArrayView();
	Uint32 index_count = lod.GetNumTriangles() * 3;
	Uint32 vertex_count = lod.PositionVertexBuffer.GetNumVertices();

	Uint16 *ia16 = new Uint16[index_count];
	for (unsigned j = 0; j < index_count / 3; ++j)	//It's strange, triangle indices are Right handed...
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
	serialize_align(ar, index_count * 2);
	ar->Serialize(pos, vertex_count * 12);
	ar->Serialize(norm, vertex_count * 12);

	new3d::geometry::CollisionConfig collisionConfig;
	memset(&collisionConfig, 0, sizeof(collisionConfig));

	if (comp->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block) {
		collisionConfig.collisionFilterMask ^= new3d::geometry::CharacterFilter;
	}

	if (comp->GetCollisionResponseToChannel(ECC_PhysicsBody) == ECR_Block) {
		collisionConfig. collisionFilterMask ^= new3d::geometry::SensorTrigger;
	}
	ar->Serialize(&collisionConfig, sizeof(collisionConfig));
	serialize_align(ar, sizeof(collisionConfig));

	SAFE_DELETE_ARRAY(ia16);
	SAFE_DELETE_ARRAY(pos);
	SAFE_DELETE_ARRAY(norm);
}

static void unit_bounds(FVector &cent, FVector &size, const FVector &tar_cent, const FVector &tar_size)
{
	FVector my_min = cent - size, my_max = cent + size;
	my_min.ComponentMin(tar_cent - tar_size);
	my_max.ComponentMax(tar_cent + tar_size);

	size = (my_max - my_min) * 0.5f;
	cent = my_min + size;
}

#define SPRAY_TEXTURE_SIZE 64
static int GetPowerLevel(int value)
{
	int level_value = 0;
	for (int i = 0; i < 64; ++i)
	{
		if (value > level_value)
		{
			level_value = 1 << (i + 1);
			continue;
		}
		return i;
	}

	return 0;
}

int CalcSprayTextureSize(float size)
{
	return (1 << GetPowerLevel(((int)size) / SPRAY_TEXTURE_SIZE)) * SPRAY_TEXTURE_SIZE;
}

void ExportLevel()
{
	FWorldContext &wc = GEditor->GetEditorWorldContext();
	UWorld *world = wc.World();

	TArray<FSprayMesh> SprayMeshs;
	UMaterial *DefaultSprayMaterial = nullptr, *WorldPosMaterial = nullptr, *WorldNormalMaterial = nullptr;
	UMaterial *CurrSprayMeshMaterial = nullptr;

	UTextureRenderTarget2D *RtHDR = nullptr, *RtLDR = nullptr;

	collectSprayMeshs(world, SprayMeshs, DefaultSprayMaterial, WorldPosMaterial, WorldNormalMaterial);
	if (SprayMeshs.Num() == 0) {
		return;
	}

	createRenderTarget(world, RtHDR, RtLDR);

	ASceneCapture2D *ch = Cast<ASceneCapture2D>(GEditor->AddActor(world->GetLevel(0), ASceneCapture2D::StaticClass(), FTransform::Identity, true, RF_Public | RF_Standalone | RF_Transactional));
	//ASceneCapture2D *ch = NewObject<ASceneCapture2D>((UObject*)world);
	if (nullptr == ch) {
		return;
	}

	USceneCaptureComponent2D *SceneCapture = ch->GetCaptureComponent2D();
	SceneCapture->ShowFlags.SetAmbientCubemap(false);
	SceneCapture->ShowFlags.SetAntiAliasing(false);
	SceneCapture->ShowFlags.SetAtmosphericFog(false);
	SceneCapture->ShowFlags.SetSkyLighting(false);
	SceneCapture->ShowFlags.SetPostProcessing(false);
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

	FString lvl_abs_path = getLevelDataPhysPath(SprayMeshs[0].actor->GetWorld());

	// Render and save WorldPos (HDR), WorldNormal (LDR).
	for (int k = 0; k < 2; ++k)
	{
		for (int i = 0; i < SprayMeshs.Num(); ++i)
		{
			FSprayMesh &mesh = SprayMeshs[i];
			FBox bbox = mesh.actor->GetComponentsBoundingBox();
			int size = MIN(1024, CalcSprayTextureSize(bbox.GetExtent().Size())/2);

			UTextureRenderTarget2D *rt = RtHDR;
			if (k == 0 || 1) 
			{
				RtHDR->InitCustomFormat(size, size, PF_FloatRGBA, false);

				SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
				SceneCapture->TextureTarget = RtHDR;
			}
			else
			{
				RtLDR->InitCustomFormat(size, size, PF_B8G8R8A8, false);

				SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
				SceneCapture->TextureTarget = RtLDR;
				rt = RtLDR;
			}

			int w = rt->GetSurfaceWidth(), h = rt->GetSurfaceHeight();
			EPixelFormat fmt = rt->GetFormat();

			// render worldpos/normal for each static-mesh actor.
			SceneCapture->ShowOnlyComponents.Empty();
			SceneCapture->ShowOnlyComponent(mesh.comp);

			// place camera to look-at the mesh actor.
			FVector cameraLoc = bbox.GetCenter() - FVector(1, 0, 0) * bbox.GetExtent().Size() * 2.0f;
			FRotator cameraRot(0, 0, 0);
			ch->SetActorLocationAndRotation(cameraLoc, cameraRot);

			// replace the mesh actor's material;			
			if (k == 0 && CurrSprayMeshMaterial != WorldPosMaterial) {
				replaceMaterial(mesh.comp, CurrSprayMeshMaterial, WorldPosMaterial, nullptr);
			}
			else if (k == 1 && CurrSprayMeshMaterial != WorldNormalMaterial) {
				replaceMaterial(mesh.comp, CurrSprayMeshMaterial, WorldNormalMaterial, nullptr);
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
			SceneCapture->CaptureScene();
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
		struct NoneSprayMesh
		{
			UStaticMeshComponent *comp;
			bool bEnableSubdMesh;
		};
		TArray<NoneSprayMesh> none_spray_meshs;
		FVector lvl_center, lvl_size;
		bool lvl_box_init = false;

		for (TActorIterator<AActor> it(world); it; ++it)
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
						if (comp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
						{
							if (!is_spraymesh) 
							{
								NoneSprayMesh noneSprayMesh;
								noneSprayMesh.comp = comp;
								noneSprayMesh.bEnableSubdMesh = spray_mesh ? spray_mesh->bEnableSubdMesh : false;
								none_spray_meshs.Add(noneSprayMesh);
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
			serialize_align(ar, 2 + name_len);

			serializeStaticMeshComponentGeom(ar, sm.comp, sm.actor->bEnableSubdMesh);
		}

		for (unsigned i = 0; i < none_spray_count; ++i) {
			serializeStaticMeshComponentGeom(ar, none_spray_meshs[i].comp, none_spray_meshs[i].bEnableSubdMesh);
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

	for (FSprayMesh &sm : SprayMeshs) {
		sm.comp->SetMaterial(0, sm.origin_mat);
	}

	world->DestroyActor(ch);
	ch = nullptr;
}
