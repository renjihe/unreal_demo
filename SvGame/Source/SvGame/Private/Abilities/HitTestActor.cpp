// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "HitTestActor.h"
//#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AHitTestActor::AHitTestActor()
{
	//PrimaryActorTick.bCanEverTick = true;

	//Age = PeriodAge = 0;
}

// Called when the game starts or when spawned
void AHitTestActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AHitTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Age += DeltaTime;
	//PeriodAge += DeltaTime;
	//
	//if (TestMode == EHitTestMode::Always || PeriodAge >= PeriodInterval) {
	//	HitTest();
	//}
}

bool AHitTestActor::InitSphere(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, float radius)
{
	ShapeType = EHitTestShape::SPHERE;
	Radius = radius;
	//Age = PeriodAge = 0;

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);
	if (nullptr == actor) {
		return false;
	}

	// create Shape.
	USphereComponent* comp = NewObject<USphereComponent>(this);
	comp->RegisterComponent();
	comp->SetSphereRadius(radius);
	RootComponent = comp;

	if (TestMode != EHitTestMode::Always) {		
		comp->SetCollisionProfileName(TEXT("NoCollision"));
	}
	else
	{
		comp->SetCollisionProfileName(TEXT("Attack"));
		comp->bReturnMaterialOnMove = true;
		comp->bTraceComplexOnMove = true;
		comp->bGenerateOverlapEvents = TestMode == EHitTestMode::Always;
		comp->OnComponentBeginOverlap.AddDynamic(this, &AHitTestActor::BeginOverlap);		
	}

	USceneComponent* attach_to = mesh ? mesh : actor->GetRootComponent();
	if (IsEffectPositionAttached(fromPosition))
	{
		comp->SetRelativeLocation(relativeLocation);
		comp->AttachToComponent(attach_to, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true), fromSocket);
	}
	else
	{
		FTransform trans = ACharacterSV::MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation);
		comp->SetRelativeLocation(trans.GetLocation());
		comp->SetRelativeRotation(trans.Rotator());
	}

	PostCreateShape();
	return true;
}

void AHitTestActor::PostCreateShape()
{
	if (TestMode != EHitTestMode::Always) {
		HitTest();
	}

	if (Duration > 0.001f) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandleDuration, this, &AHitTestActor::OnTimerLifeEnd, Duration);
	}

	if ((TestMode == EHitTestMode::Period || TestMode == EHitTestMode::PeriodOnce) && PeriodInterval > 0.001f) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandlePeriod, this, &AHitTestActor::OnTimerPeriod, PeriodInterval);
	}

	if (DrawDebug) {
		//PrimaryActorTick.bCanEverTick = true;
		RootComponent->SetHiddenInGame(false);
	}
}

void AHitTestActor::HitTest()
{
	//PeriodAge = 0;

	TArray<TEnumAsByte<EObjectTypeQuery>> types;
	TEnumAsByte<EObjectTypeQuery> t(TRACE_FIRE);
	types.Add(t);

	TArray<AActor*> ignore;

	TArray<UPrimitiveComponent*> overlapped;
	UKismetSystemLibrary::SphereOverlapComponents(this, GetActorLocation(), Radius, types, UMeshComponent::StaticClass(), ignore, overlapped);

	for (int j = 0; j < overlapped.Num(); ++j)
	{
		AActor *target = overlapped[j]->GetOwner();
		if (OwnerAbility->FilterHitTarget(target, TargetType)) {
			continue;
		}

		
		TArray<FHitResult> hitArr;
		EObjectTypeQuery objectType = UEngineTypes::ConvertToObjectType(overlapped[j]->GetCollisionObjectType());

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(objectType);
		if (UKismetSystemLibrary::LineTraceMultiForObjects(this, GetActorLocation(), overlapped[j]->GetComponentLocation(), types, false, ignore, EDrawDebugTrace::None, hitArr, false))
		{
			for (int k = 0; k < hitArr.Num(); ++k)
			{
				if (overlapped[j] == hitArr[k].GetComponent()) {
					OwnerAbility->HitTarget(hitArr[k]);
					break;
				}
			}
		}
		else {
			FHitResult hit(target, overlapped[j], overlapped[j]->GetComponentLocation(), overlapped[j]->GetComponentRotation().Vector());
			OwnerAbility->HitTarget(hit);
		}
		
	}
}

void AHitTestActor::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool FromSweep, const FHitResult &SweepResult)
{
	if (!OwnerAbility->FilterHitTarget(OtherActor, TargetType)) 
	{
		TArray<FHitResult> hitArr;
		TArray<AActor*> ignore;
		EObjectTypeQuery objectType = UEngineTypes::ConvertToObjectType(OtherComp->GetCollisionObjectType());

		TArray<TEnumAsByte<EObjectTypeQuery>> types;
		types.Add(objectType);
		if (UKismetSystemLibrary::LineTraceMultiForObjects(this, GetActorLocation(), OtherComp->GetComponentLocation(), types, false, ignore, EDrawDebugTrace::None, hitArr, false))
		{
			for (int k = 0; k < hitArr.Num(); ++k)
			{
				if (OtherComp == hitArr[k].GetComponent()) {
					OwnerAbility->HitTarget(hitArr[k]);
					break;
				}
			}
		}
		else {
			OwnerAbility->HitTarget(SweepResult);
		}
		
	}	
}

void AHitTestActor::OnTimerLifeEnd()
{
	OwnerAbility->DestroyHitTestActor(this);
}

void AHitTestActor::OnTimerPeriod()
{
	HitTest();
}