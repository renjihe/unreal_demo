// Fill out your copyright notice in the Description page of Project Settings.

#include "SprayMgr.h"
#include "SpGame.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HighResScreenshot.h"

#include "StaticMeshResources.h"

#include "SpGameModeBattle.h"
#include "SpStaticMeshActor.h"
#include "SpGameInstance.h"
#include "client/dllclient.h"

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

ASprayMgr *GSprayMgr = nullptr;

extern void SetCurrSpWorld(UWorld *world);

ASprayMgr::ASprayMgr(const FObjectInitializer &objInitor)
	: Super(objInitor), SprayMode(0)
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

	static ConstructorHelpers::FObjectFinder<UMaterial> mat(TEXT("/Game/ExcuteSprayMaterial.ExcuteSprayMaterial"));
	ExcuteSpayMaterial = mat.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> mat2(TEXT("/Game/Spray2MeshMaterial.Spray2MeshMaterial"));
	Spray2MeshMaterial = mat2.Object;
}

void ASprayMgr::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld *world = GetWorld();
	if (world && world->GetGameInstance() && Cast<USpGameInstance>(world->GetGameInstance())) 
	{
		GSprayMgr = this;
		SetCurrSpWorld(world);
	}	
}

static void InitPhysic(AStaticMeshActor *actor)
{
	TArray<UActorComponent *> comps = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
	for (int j = 0; j < comps.Num(); ++j)
	{
		UStaticMeshComponent *comp = Cast<UStaticMeshComponent>(comps[j]);
		if (comp)
		{
			ECollisionResponse cameraCollisionResponse = comp->GetCollisionResponseToChannel(ECC_Camera);
			comp->SetCollisionObjectType(ECC_WorldStatic);
			comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			comp->SetCollisionResponseToAllChannels(ECR_Ignore);
			comp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			comp->SetCollisionResponseToChannel(ECC_Camera, cameraCollisionResponse);
		}
	}
}

FString getLevelDataEnginePath(UWorld *world);
void ASprayMgr::BeginPlay()
{
	Super::BeginPlay();

	FVector AtkerColor;
	FVector DeferColor;
	GetSprayColor(AtkerColor, DeferColor);

	UWorld *world = GetWorld();
	FString lvl_path = getLevelDataEnginePath(world);
	UMaterial *defaultSprayMaterial = LoadObject<UMaterial>(world, TEXT("/Game/SprayMeshMaterial.SprayMeshMaterial"));

#if WITH_EDITOR
	UMaterial *debugMaterial = LoadObject<UMaterial>(world, TEXT("/Game/DebugTextureMaterial.DebugTextureMaterial"));
	UMaterialInstanceDynamic *debugMaterialDyna = nullptr;
	UTextureRenderTarget2D *debugRT = nullptr;
#endif

	// filter ASpStaticMeshActor, replace as SprayMeshMaterial instance, and initialize FSprayMeshActor for it;
	for (TActorIterator<AActor> it(world); it; ++it)
	{
		AStaticMeshActor *static_actor = Cast<AStaticMeshActor>(*it);
		if (static_actor) {
			InitPhysic(static_actor);
		}

		ASpStaticMeshActor *actor = Cast<ASpStaticMeshActor>(*it);
		if (nullptr == actor || (!actor->bEnableSpray && !actor->ActorHasTag("Defer") && !actor->ActorHasTag("Atker"))) {
			continue;
		}

		FSprayMeshActor sma;
		sma.actor = actor;
		sma.comp = nullptr;
		sma.material = nullptr;
		sma.sprayCount = 0;
		sma.rtDataDirty = 0; 
		sma.atkerSprayScore = 0;
		sma.deferSprayScore = 0;

		FBox bbox = sma.actor->GetComponentsBoundingBox();
		sma.size = MIN(2048, CalcSprayTextureSize(bbox.GetExtent().Size())/2);

		TArray<UActorComponent *> comps = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (int j = 0; j < comps.Num() && sma.comp == nullptr; ++j)
		{
			UStaticMeshComponent *comp = Cast<UStaticMeshComponent>(comps[j]);

			TArray<class UMaterialInterface *> materials = comp->GetMaterials();
			for (int i = 0; i < materials.Num(); ++i)
			{
				if (materials[i] /*&& materials[i]->GetBaseMaterial() == defaultSprayMaterial*/)
				{
					UMaterialInstanceDynamic *mat = UMaterialInstanceDynamic::Create(0 == SprayMode ? materials[i] : Spray2MeshMaterial, actor);
					if (mat)
					{
						sma.material = mat;
						sma.comp = comp;
						mat->SetVectorParameterValue(TEXT("AtkerColor"), AtkerColor);
						mat->SetVectorParameterValue(TEXT("DeferColor"), DeferColor);

						if (actor->ActorHasTag("Defer")) {
							mat->SetScalarParameterValue(TEXT("Fac"), 1);
						}

						comp->SetMaterial(i, mat);
						break;
					}
				}
			}			
		}

		if (sma.comp != nullptr)
		{
			const FName TargetName = MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), TEXT("ASprayMgrSceneCapture"));
			sma.rt = NewObject<UTextureRenderTarget2D>(this, TargetName);
			sma.rt->InitCustomFormat(sma.size, sma.size, PF_B8G8R8A8, false);
			sma.rt->ClearColor = FLinearColor(0, 0, 0, 0);
			sma.rt->AddToRoot();
			sma.material->SetTextureParameterValue("TexSprayRenderTarget", sma.rt);
			sma.material->SetScalarParameterValue("TexSprayRenderTargetTexel", 1.0f / sma.size);

			sma.rt2 = NewObject<UTextureRenderTarget2D>(this);
			sma.rt2->InitCustomFormat(sma.size, sma.size, PF_B8G8R8A8, false);
			sma.rt2->ClearColor = FLinearColor(0, 0, 0, 0);
			sma.rt2->AddToRoot();

#if WITH_EDITOR
			if (nullptr == debugRT) {
				debugRT = sma.rt;
			}
#endif

			FString path = lvl_path + sma.actor->GetFName().ToString() + TEXT("_WorldPos");
			sma.worldpos = LoadObject<UTexture2D>(this, *path);
			if (sma.worldpos) {
				sma.worldpos->AddToRoot();
			}

			if (0 != SprayMode)
			{
				FStaticMeshLODResources &lod = sma.comp->GetStaticMesh()->RenderData->LODResources[0];
				if (sma.comp->LODData.Num() == 0) {
					sma.comp->SetLODDataCount(1, 0);
				}
				FStaticMeshComponentLODInfo *lod_info = &sma.comp->LODData[0];
				lod_info->OverrideVertexColors = new FColorVertexBuffer;
				lod_info->OverrideVertexColors->InitFromSingleColor(FColor::Black, lod.GetNumVertices());

				BeginInitResource(lod_info->OverrideVertexColors);
				sma.comp->MarkRenderStateDirty();
			}
			SprayMeshActors.Add(sma);			
		}
	}

	// set component to geometry object.
	for (FSprayMeshActor &sma : SprayMeshActors) 
	{
		sma.actor->SprayContext = &sma;
		new3d::geometry::CollisionUserData collision;
		collision.data = sma.comp;
		collision.type = new3d::geometry::CUDTYPE_SPRAY_STATIC_OBJECT;
		sma.physicActor = spclient::coupleBattleActorGeometry(TCHAR_TO_UTF8(*sma.actor->GetFName().ToString()), sma.comp, &collision);
		sma.totalSprayScore = spclient::getTotalSprayScore(sma.physicActor);
	}

	// initialize DebugTextureMaterial in level actors;
#if WITH_EDITOR
	for (TActorIterator<AActor> it(world); it; ++it)
	{
		AStaticMeshActor *actor = Cast<AStaticMeshActor>(*it);
		if (nullptr == actor) {
			continue;
		}

		TArray<UActorComponent *> comps = actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (int j = 0; j < comps.Num(); ++j)
		{
			UStaticMeshComponent *comp = Cast<UStaticMeshComponent>(comps[j]);

			TArray<class UMaterialInterface *> materials = comp->GetMaterials();
			for (int i = 0; i < materials.Num(); ++i)
			{
				if (materials[i] && materials[i]->GetBaseMaterial() == debugMaterial)
				{
					if (nullptr == debugMaterialDyna) {
						debugMaterialDyna = UMaterialInstanceDynamic::Create(materials[i], actor);
					}

					if (debugMaterialDyna) {
						comp->SetMaterial(i, debugMaterialDyna);
					}
				}
			}
		}
	}

	if (debugMaterialDyna && debugRT) {
		debugMaterialDyna->SetTextureParameterValue("TexDebug", debugRT);
	}
#endif
}

void ASprayMgr::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (FSprayMeshActor &sma : SprayMeshActors)
	{
		if (sma.rt) {
			sma.rt->RemoveFromRoot();
		}

		if (sma.rt2) {
			sma.rt2->RemoveFromRoot();
		}

		if (sma.worldpos) {
			sma.worldpos->RemoveFromRoot();
		}	
	}
	SprayMeshActors.Empty();

	if (this == GSprayMgr) 
	{
		GSprayMgr = nullptr;
		SetCurrSpWorld(nullptr);
	}
}

void ASprayMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASprayMgr::DoSpray2(FVector loc, FVector normal, FVector dir, int faction, USprayMask *mask, FVector size)
{
	if (nullptr == GSprayMgr || GSprayMgr->SprayMode != 1) {
		return;
	}

	FVector U = FVector::CrossProduct(normal, dir);	//x
	U.Normalize();
	FVector V = FVector::CrossProduct(dir, U); //y
	
	FBox sprayBox = FBox::BuildAABB(loc, size / 2);
	for (int j = 0; j < GSprayMgr->SprayMeshActors.Num(); ++j)
	{
		FSprayMeshActor &sma = GSprayMgr->SprayMeshActors[j];
		FBox bbox = sma.actor->GetComponentsBoundingBox();

		if (bbox.Intersect(sprayBox))
		{
			bool sprayed = false;
			UStaticMesh *mesh = sma.comp->GetStaticMesh();
			const FTransform &world = sma.comp->GetComponentTransform();
			FStaticMeshComponentLODInfo &lod_info = sma.comp->LODData[0];
			FStaticMeshLODResources &lod = sma.comp->GetStaticMesh()->RenderData->LODResources[0];

			Uint32 vertex_count = lod.PositionVertexBuffer.GetNumVertices();
			for (unsigned j = 0; j < vertex_count; ++j) 
			{
				//FVector pos = world.TransformPosition(lod.PositionVertexBuffer.VertexPosition(j));
				//FVector n = world.TransformVectorNoScale(lod.VertexBuffer.VertexTangentZ(j));
				FVector pos = lod.PositionVertexBuffer.VertexPosition(j);
				FVector n = lod.VertexBuffer.VertexTangentZ(j);

				FVector d = pos - loc;
				float dis_to_hitplane = FVector::DotProduct(d, normal);
				float u = FVector::DotProduct(U, d - n*dis_to_hitplane) / size.X;
				if (u > -1 && u < 1)
				{
					float v = FVector::DotProduct(V, d - n*dis_to_hitplane) / size.Y;
					if (v > -1 && v < 1)
					{
						int weight = mask->GetMask(u, v);
						if (weight > 0) 
						{
							FColor &clr = lod_info.OverrideVertexColors->VertexColor(j);
							if (faction == 0) 
							{
								if (clr.R < weight) 
								{
									clr.R = weight;
									if (clr.G > 255 - weight) {
										clr.G = 255 - weight;
									}
								}
							}
							else
							{
								if (clr.G < weight)
								{
									clr.G = weight;
									if (clr.R > 255 - weight) {
										clr.R = 255 - weight;
									}
								}							
							}

							if (!sprayed) {
								sprayed = true;
							}
						}
					}
				}				
			}

			if (sprayed) {
				BeginUpdateResourceRHI(lod_info.OverrideVertexColors);
			}
		}				
	}
}

void ASprayMgr::Spray(int caster, FVector loc, FVector normal, FVector dir, int faction, UTexture2D *mask, FVector size, float range)
{
	if (GSprayMgr)
	{
		UMaterialInstanceDynamic *mat = GSprayMgr->NewExecuteSprayMaterial();
		if (mat)
		{
			FVector az = normal;
			FVector ax = dir;
			if (abs(FVector::DotProduct(ax, az)) > 0.99f) 
			{
				if (abs(az.X) < 0.99f) {
					ax.Set(1, 0, 0);
				}
				else {
					ax.Set(0, 1, 0);
				}				
			}
			FVector ay = FVector::CrossProduct(az, ax);
			ay.Normalize();

			FMatrix scl = FScaleMatrix::Make(size);
			FMatrix trans = FRotationMatrix::MakeFromYZ(ay, az);
			trans.SetOrigin(loc);

			FMatrix m = (scl * trans).Inverse();

			FVector dir2d(dir.X, dir.Y, 0);
			if (!dir2d.Normalize()) {
				dir2d.Set(1, 0, 0);
			}

			FColor factionColor = faction == 0 ? FColor(255, 0, 0, 255) : FColor(0, 255, 0, 255);

			mat->SetTextureParameterValue("TexMask", mask);
			mat->SetVectorParameterValue("Loc", loc);
			mat->SetVectorParameterValue("Dir", dir2d);
			mat->SetVectorParameterValue("Size", size);
			mat->SetVectorParameterValue("Fac", factionColor);
			mat->SetVectorParameterValue("TransX", FLinearColor(m.M[0][0], m.M[1][0], m.M[2][0], m.M[3][0]));
			mat->SetVectorParameterValue("TransY", FLinearColor(m.M[0][1], m.M[1][1], m.M[2][1], m.M[3][1]));
			mat->SetVectorParameterValue("TransZ", FLinearColor(m.M[0][2], m.M[1][2], m.M[2][2], m.M[3][2]));

			FBox sprayBox = FBox::BuildAABB(loc, size/2);
			for (int j = 0; j < GSprayMgr->SprayMeshActors.Num(); ++j)
			{
				FSprayMeshActor &sma = GSprayMgr->SprayMeshActors[j];
				FBox bbox = sma.actor->GetComponentsBoundingBox();

				if (bbox.Intersect(sprayBox))
				{
					mat->SetVectorParameterValue("BBoxMin", bbox.Min);
					mat->SetTextureParameterValue("TexWorldPos", sma.worldpos);
					mat->SetTextureParameterValue("TexSprayRenderSource", sma.rt);
					ExecuteSpray(sma.rt2, mat);

					ExecuteSprayVertex(caster, sma, faction, m.M, range);

					SwapTextureRenderTarget(sma);

					++sma.sprayCount;
				}
			}			
		}		
	}	
}

void ASprayMgr::ExecuteSprayVertex(int caster, FSprayMeshActor &sma, int faction, float(*m)[4], float range)
{
	spclient::executeSprayVertex(sma.physicActor, faction, m, range, sma.atkerSprayScore, sma.deferSprayScore);
	AGameModeBattle *GameMode = nullptr;
	if (GSpWorld) 
	{
		GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
		if (GameMode)
		{
			float OldScore = faction ? GameMode->DeferScore : GameMode->AtkerScore;
			UpdateScore(sma);
			float Score = faction ? GameMode->DeferScore : GameMode->AtkerScore;
			spclient::updateMpEx(caster, Score - OldScore);
		}		
	}
}

void ASprayMgr::ExecuteSpray(UTextureRenderTarget2D *rt, UMaterialInstanceDynamic *mat)
{
	UWorld *world = GSprayMgr->GetWorld();
	UCanvas *canvas = nullptr;
	FVector2D canvasSize;
	FDrawToRenderTargetContext context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(world, rt, canvas, canvasSize, context);
	if (canvas)
	{
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(world, rt, mat);
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(world, context);
	}	
}

void ASprayMgr::SwapTextureRenderTarget(FSprayMeshActor &sma)
{
	auto t = sma.rt;
	sma.rt = sma.rt2;
	sma.rt2 = t;
	sma.material->SetTextureParameterValue("TexSprayRenderTarget", sma.rt);
}

FColor ASprayMgr::ReadSprayRenderTarget(FSprayMeshActor &sma, const FVector2D &uv)
{
	if (sma.rtDataDirty != sma.sprayCount)
	{
		FTextureRenderTargetResource *rtResource = sma.rt->GameThread_GetRenderTargetResource();

		FHighResScreenshotConfig& ssConfig = GetHighResScreenshotConfig();
		rtResource->ReadPixels(sma.rtData);
		sma.rtDataDirty = sma.sprayCount;
	}

	int x = (uv.X - floor(uv.X)) * sma.size + 0.5f;
	int y = (uv.Y - floor(uv.Y)) * sma.size + 0.5f;
	int index = x + y * sma.size;
	return index < sma.rtData.Num() ? sma.rtData[index] : FColor();
}

void ASprayMgr::UpdateScore(FSprayMeshActor &sma)
{
	AGameModeBattle *GameMode = nullptr;
	if (GSpWorld) {
		GameMode = Cast<AGameModeBattle>(GSpWorld->GetAuthGameMode());
	}

	if (GameMode)
	{
		int atkerScore = 0;
		int deferScore = 0;
		int totalScore = 0;
		for (int j = 0; j < GSprayMgr->SprayMeshActors.Num(); ++j)
		{
			FSprayMeshActor &actor = GSprayMgr->SprayMeshActors[j];
			atkerScore += actor.atkerSprayScore;
			deferScore += actor.deferSprayScore;
			totalScore += actor.totalSprayScore;
		}

		GameMode->AtkerScore = atkerScore * 1.0f / totalScore;
		GameMode->DeferScore = deferScore * 1.0f / totalScore;
	}
}

UMaterialInstanceDynamic* ASprayMgr::NewExecuteSprayMaterial()
{
	if (nullptr == ExcuteSpayMaterialDynamic) {
		ExcuteSpayMaterialDynamic = UMaterialInstanceDynamic::Create(ExcuteSpayMaterial, this);
	}
	return ExcuteSpayMaterialDynamic;

	UMaterialInstanceDynamic *mat = nullptr;
	if (ExecuteSprayMaterialPool.Num() == 0)
	{
		mat = UMaterialInstanceDynamic::Create(ExcuteSpayMaterial, this);
		if (mat) {
			mat->AddToRoot();
		}
	}
	else {
		mat = ExecuteSprayMaterialPool.Pop();
	}

	return mat;
}

void ASprayMgr::DelExecuteSprayMaterial(UMaterialInstanceDynamic *mat)
{
	ExecuteSprayMaterialPool.Add(mat);
}

void ASprayMgr::CleanExecuteSprayMaterial()
{
	for (auto mat : ExecuteSprayMaterialPool) {
		mat->RemoveFromRoot();
	}

	ExecuteSprayMaterialPool.Empty();
}


extern bool CalcWorldPointUV(UStaticMeshComponent *compo, int faceIndex, FVector worldLocation, FVector2D &outUV);

int calc_spray2_faction(FVector hit, int face, UStaticMeshComponent *compo);
int calc_spray_faction(FVector hit, int face, UStaticMeshComponent *compo)
{
	int faction = 0;
	FVector2D uv(0, 0);
	if (compo && CalcWorldPointUV(compo, face, hit, uv))
	{
		ASpStaticMeshActor *actor = Cast<ASpStaticMeshActor>(compo->GetOwner());
		if (actor && actor->SprayContext)
		{
			FColor clr = ASprayMgr::ReadSprayRenderTarget(*actor->SprayContext, uv);
			if (clr.R > 126) {
				faction = 1;
			}
			else if (clr.G > 126) {
				faction = 2;
			}
		}
	}
	return faction;
}

int ASprayMgr::GetSprayFaction(FVector rayOrigin, FVector rayDir)
{
	return 0;
	int fac = 0;

	TArray<AActor*> ignore;
	FHitResult hit;
	if (UKismetSystemLibrary::LineTraceSingle(GSprayMgr, rayOrigin, rayOrigin + rayDir*10000, (ETraceTypeQuery)ECC_WorldStatic, true, ignore, EDrawDebugTrace::None, hit, true))
	{
		UStaticMeshComponent *compo = Cast<UStaticMeshComponent>(hit.GetComponent());
		if (GSprayMgr->SprayMode == 0) {
			fac = calc_spray_faction(hit.Location, hit.FaceIndex, compo);
		}
		else {
			fac = calc_spray2_faction(hit.Location, hit.FaceIndex, compo);
		}		
	}

	return fac;
}

FSprayMeshActor* ASprayMgr::FindSprayMeshActor(AActor *actor)
{
	for (FSprayMeshActor &sma : SprayMeshActors)
	{
		if (sma.actor == actor) {
			return &sma;
		}
	}
	return nullptr;
}

extern void SaveTexture2D(UTexture2D *tex, FString path);
extern void SaveRenderTarget2D(UTextureRenderTarget2D *rt, FString path);
void ASprayMgr::SaveSprayTexture()
{
	if (GSprayMgr)
	{
		for (FSprayMeshActor &sma : GSprayMgr->SprayMeshActors)
		{
			if (sma.rt)
			{
				SaveRenderTarget2D(sma.rt, TEXT("d:\\dev_head\\rt.png"));
			}
		}
	}
}

#define FVECTOR(arr) FVector(arr[0], arr[1], arr[2])

void spray_callback(const float *loc, const float *normal, const float *dir, int faction, const char *mask, const float *size)
{
	if (GSprayMgr)
	{
		UTexture2D *tex = LoadObject<UTexture2D>(GSprayMgr, UTF8_TO_TCHAR(mask));
		ASprayMgr::Spray(0, FVECTOR(loc), FVECTOR(normal), FVECTOR(dir), faction, tex, FVECTOR(size));
	}	
}

int get_spray_faction_callback(const float *pt, int face, void *geomUserdata)
{
	return calc_spray_faction(FVECTOR(pt), face, (UStaticMeshComponent *)geomUserdata);
}

static void add_neighbor(FVertexNeighborInfo &ni, unsigned neighbor)
{
	if (ni.count >= ARRAY_LEN(ni.neighbor)) {
		return;
	}

	for (unsigned i = 0; i < ni.count; ++i)
	{
		if (ni.neighbor[i].index == neighbor) {
			return;
		}
	}

	ni.neighbor[ni.count++].index = neighbor;
}

static void calc_neighbor_weight(FVertexNeighborInfo &ni, unsigned i, FStaticMeshLODResources &lod)
{
	FVector v0 = lod.PositionVertexBuffer.VertexPosition(i);
	FVector tanx = lod.VertexBuffer.VertexTangentX(i);
	FVector tany = lod.VertexBuffer.VertexTangentY(i);

	float wu[ARRAY_LEN(ni.neighbor)], wv[ARRAY_LEN(ni.neighbor)];
	float wut = 0, wvt = 0, wu0 = 1, wv0 = 1;
	for (unsigned j = 0; j < ni.count; ++j)
	{
		FVector vj = lod.PositionVertexBuffer.VertexPosition(ni.neighbor[j].index);
		FVector d = vj - v0;
		wu[j] = FVector::DotProduct(d, tanx);
		wv[j] = FVector::DotProduct(d, tany);

		if (j == 0) 
		{
			float wuj = wu[j] < 0 ? -wu[j] : wu[j];
			float wvj = wv[j] < 0 ? -wv[j] : wv[j];
			wu0 = wuj;
			wv0 = wvj;
			if (wu0 < 0.001f) {
				wu0 = 0.001f;
			}
			if (wv0 < 0.001f) {
				wv0 = 0.001f;
			}

			wu[0] = wu[j] < 0 ? -1 : 1;
			wv[0] = wv[j] < 0 ? -1 : 1;
		}
		else 
		{
			if (wu[j] > 0 && wu[j] < 0.001f) {
				wu[j] = 0.001f;
			}
			else if (wu[j] < 0 && wu[j] > -0.001f) {
				wu[j] = -0.001f;
			}

			if (wv[j] > 0 && wv[j] < 0.001f) {
				wv[j] = 0.001f;
			}
			else if (wv[j] < 0 && wv[j] > -0.001f) {
				wv[j] = -0.001f;
			}
			
			wu[j] = wu0 / wu[j];
			wv[j] = wv0 / wv[j];
		}

		wut += N3ABS(wu[j]);
		wvt += N3ABS(wv[j]);
	}

	for (unsigned j = 0; j < ni.count; ++j)
	{
		ni.neighbor[i].wu = (char)(wu[i] / wut * 127);
		ni.neighbor[i].wv = (char)(wv[i] / wut * 127);
	}
}

void FSprayMeshActor::buildVertexNeighborInfo()
{
	FStaticMeshLODResources &lod = comp->GetStaticMesh()->RenderData->LODResources[0];
	const FIndexArrayView &ia = lod.IndexBuffer.GetArrayView();
	unsigned triangles = lod.GetNumTriangles();
	unsigned vertecis = lod.PositionVertexBuffer.GetNumVertices();

	// build neighbor connection
	vertex_info.SetNumZeroed(vertecis);
	for (unsigned i = 0; i < triangles; ++i)
	{
		unsigned v0 = ia[i * 3], v1 = ia[i * 3 + 1], v2 = ia[i * 3 + 2];
		add_neighbor(vertex_info[v0], v1);
		add_neighbor(vertex_info[v0], v2);

		add_neighbor(vertex_info[v1], v0);
		add_neighbor(vertex_info[v1], v2);

		add_neighbor(vertex_info[v2], v0);
		add_neighbor(vertex_info[v2], v1);
	}

	// build neighbor weight
	for (unsigned i = 0; i < vertecis; ++i)	{
		calc_neighbor_weight(vertex_info[i], i, lod);
	}
}

int calc_spray2_faction(FVector hit, int face, UStaticMeshComponent *compo)
{
	int faction = 0;
	if (compo && compo->LODData.Num() && compo->LODData[0].OverrideVertexColors)
	{
		UStaticMesh *mesh = compo->GetStaticMesh();
		ASpStaticMeshActor *actor = Cast<ASpStaticMeshActor>(compo->GetOwner());
		if (actor && mesh)
		{
			const FStaticMeshLODResources &lod = mesh->GetLODForExport(0);
			FColorVertexBuffer *vertex_color = compo->LODData[0].OverrideVertexColors;

			int triangles = lod.GetNumTriangles();

			const FIndexArrayView &ia = lod.IndexBuffer.GetArrayView();
			if (face * 3 + 2 < ia.Num())
			{
				const FColor &c0 = vertex_color->VertexColor(ia[face * 3]), &c1 = vertex_color->VertexColor(ia[face * 3 + 1]), &c2 = vertex_color->VertexColor(ia[face * 3 + 2]);
				if (c0.R > 200)
				{
					if (c1.R > 200 && c2.R > 200) {
						faction = 1;
					}
				}
				else if (c0.G > 200 && c1.G > 200 && c2.G > 200) {
					faction = 2;
				}
			}
		}
	}

	return faction;
}