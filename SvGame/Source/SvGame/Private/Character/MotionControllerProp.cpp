// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "MotionControllerProp.h"
#include "Props/GraspedProp.h"
#include "Abilities/AbilityEntity.h"
#include "PlayerControllerSV.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include <UnrealNetwork.h>

AMotionControllerProp::AMotionControllerProp():
	AMotionControllerSV()
{

/*	_TouchCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TouchCollision"));
	_TouchCollision->SetupAttachment(MotionController);*/

	GripPhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("GripPhysicsHandle"));

	IsGripping = 0;	
	GrabFraceSphereRadius = 10;
	PullDistance = 500;
	IsPressing = false;
	PressingTime = 0;

	PropsInfo.Reset();
	for (int i = 0; i < EPropsType::Count; ++i)
	{
		PropsInfo.Add(FGraspedPropInfo());
	}

}

// Called when the game starts or when spawned
void AMotionControllerProp::BeginPlay()
{
	Super::BeginPlay();

/*	if (_TouchCollision)
	{
		//_TouchCollision->OnComponentBeginOverlap.AddDynamic(this, &AMotionControllerProp::TouchCollisionBeginOverlap);
		//_TouchCollision->OnComponentEndOverlap.AddDynamic(this, &AMotionControllerProp::TouchCollicionEndOverlap);
	}*/

	if (GetOwner())
	{
		if (GetOwner()->Role == ROLE_Authority)
		{
			CreateDefaultProp();
		}
	}

}

void AMotionControllerProp::CreateDefaultProp()
{
	//创建默认道具
	if (DefaultProp != nullptr)
	{
		CurrGripTarget = GetWorld()->SpawnActor<AGraspedProp>(DefaultProp, FVector::ZeroVector, FRotator::ZeroRotator);
		CurrGripTarget->SetActorTransform(CalculateNewTransform(false));
		CurrGripTarget->PickedUpStart(Instigator);
		

		APlayerControllerSV *myController = Cast<APlayerControllerSV>(GetWorld()->GetFirstPlayerController());
		if (myController) {
			myController->ServerReplicateTrans(CurrGripTarget);
		}

		EPropsType::Type PropType = CurrGripTarget->GetPropType();
		CurrGripTarget->ChangeData.AddDynamic(this, &AMotionControllerProp::ChangeData);
		ChangeData(CurrGripTarget);

		GripTime = 0.0f;
		IsGripping = false;
		FollowMotion = true;

		OnRep_CurGriptarget();
	}
}

void AMotionControllerProp::OnRep_CurGriptarget()
{
	ViveMesh->SetHiddenInGame((CurrGripTarget != nullptr), false);
}

// Called every frame
void AMotionControllerProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsMoveAimRotator)
	{
		if (AimCurve)
		{
			float fMinTime, fMaxTime;
			AimCurve->GetTimeRange(fMinTime, fMaxTime);
			if (MoveAimTime >= fMaxTime)
			{
				MoveAimTime = 0.0f;
				IsMoveAimRotator = false;
			}
			else
			{
				float fAimValue = GripCurve->GetFloatValue(MoveAimTime);
				FRotator curRot = RelativeTransfrom.GetRotation().Rotator();
				RelativeTransfrom.SetRotation(FQuat(curRot + fAimValue * (GoldAimRotatior - curRot)));

				MoveAimTime += DeltaTime;
			}
		}
		else
		{
			RelativeTransfrom.SetRotation(FQuat(GoldAimRotatior));
			IsMoveAimRotator = false;
			MoveAimTime = 0.0f;
		}
	}

	ViveMesh->SetRelativeTransform(RelativeTransfrom);

	ACharacter *ch = Cast<ACharacter>(GetOwner());
	if (ch)
	{
		if (CurrGripTarget/* && ch->Role == ROLE_Authority*/)
		{
			CalcGripTargetTransform(DeltaTime);
		}
		
		if (ch->IsLocallyControlled())
		{
			TraceForPickups();
		}
	}	

	ACharacterSV *chsv = Cast<ACharacterSV>(GetOwner());
	if (IsPressing)
	{
		PressingTime += DeltaTime;
		if (PressingTime > 1.5 && chsv)
		{
			chsv->AbilityTouchOff(EAbilityBar::BulletHP, true);
		}
	}
}
/*
void AMotionControllerProp::TouchCollisionBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	//若碰到的物品是手里的,或者是已经有触碰物体了，就不处理
	if (OtherActor == CurrGripTarget || TraceActor != nullptr) {
		return;
	}
	
	bool bTouching = false;
	//此处处理道具物品
	if (OtherActor->GetClass()->ImplementsInterface(UPropsInterface::StaticClass())) 
	{
		//可被抓取
		if (IPropsInterface::Execute_CanBePackup(OtherActor))
		{	
			TraceActor = OtherActor;
			IPropsInterface::Execute_SelectedByGrabActor(OtherActor, true);
		}
	}
}

void AMotionControllerProp::TouchCollicionEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//若有触碰物，并且触碰物是道具
	if (TraceActor != nullptr && TraceActor->GetClass()->ImplementsInterface(UPropsInterface::StaticClass()))
	{
		IPropsInterface::Execute_SelectedByGrabActor(TraceActor, false);
		TraceActor = nullptr;
	}
}
*/
void AMotionControllerProp::TraceForPickups()
{
	FVector StartTrace = ViveMesh->GetSocketLocation(TraceSocket);//GetComponentLocation();
	FVector EndTrace = StartTrace + UKismetMathLibrary::GetForwardVector(ViveMesh->GetSocketRotation(TraceSocket)) * PullDistance;

	TArray<FHitResult> hits;
	TArray<AActor*> ingoreActor;
	bool noTrace = true;
	if (UKismetSystemLibrary::SphereTraceMulti(this, StartTrace, EndTrace, GrabFraceSphereRadius, TRACE_PROP, false, ingoreActor, EDrawDebugTrace::None, hits, true))
	{
		for (int i = 0; i < hits.Num(); ++i)
		{
			AGraspedProp* graspActor = Cast<AGraspedProp>(hits[i].GetActor());
			if (graspActor)
			{

				if (graspActor == TraceActor)
				{
					noTrace = false;
					break;
				}
				else
				{
					if (!graspActor->CanBePackup()) {
						continue;
					}
					noTrace = false;
 					if (TraceActor) {
						TraceActor->SelectedByGrabActor(false);
					}

					TraceActor = graspActor;
					TraceActor->SelectedByGrabActor(true);
					break;
				}
			}
		}
		
	}
	if (noTrace)
	{
		if (TraceActor) {
			TraceActor->SelectedByGrabActor(false);
		}
		TraceActor = nullptr;
	}
}

void AMotionControllerProp::TryGripTarget(AGraspedProp *target)
{
	if (GetOwner())
	{
		if (GetOwner()->Role == ROLE_Authority) {
			GripTarget(target);
		}
		else {
			ServerGripTarget(target);
		}
	}	
}

bool AMotionControllerProp::ServerGripTarget_Validate(AGraspedProp *target)
{
	return true;
}

void AMotionControllerProp::ServerGripTarget_Implementation(AGraspedProp *target)
{
	ClientGripTarget(GripTarget(target));
	/*if (GripTarget(target)) {
		NetMulticastGripTarget(target);
	}*/
}

void AMotionControllerProp::ClientGripTarget_Implementation(bool grib)
{
	if (grib)
	{
		TraceActor->SelectedByGrabActor(false);
		TraceActor = nullptr;
		ViveMesh->SetHiddenInGame(true, false);
	}
}

void AMotionControllerProp::NetMulticastGripTarget_Implementation(AGraspedProp *target)
{
	//TraceActor = nullptr;
	//ViveMesh->SetHiddenInGame(true, false);
	//GripTarget(target);
}


void AMotionControllerProp::SeamlessTravelCleanUp()
{
	TraceActor = nullptr;
}

bool AMotionControllerProp::GripTarget(AGraspedProp *target)
{
	if (target == CurrGripTarget) {
		return false;
	}

	//若没有触碰物体，则扔掉手中的道具
	if (target == nullptr)
	{
		if (CurrGripTarget->CanThrownAway())
		{
			EPropsType::Type PropType = CurrGripTarget->GetPropType();
			int index = int(PropType);
			if (index < PropsInfo.Num())
			{
				PropsInfo[index] = FGraspedPropInfo();
			}
			FollowMotion = false;
			CurrGripTarget->ThrownAway();
			CurrGripTarget->ChangeData.RemoveDynamic(this, &AMotionControllerProp::ChangeData);
			CurrGripTarget = nullptr;
			GripPhysicsHandle->ReleaseComponent();

			if (PropsInfo[int(EPropsType::PrimaryWeapon)].PropClass != nullptr) {
				ChangeProp(EPropsType::PrimaryWeapon);
			}
			/*else {
				ViveMesh->SetHiddenInGame(false, false);
			}*/
			
		}
	}
	else
	{
		if (!target->CanBePackup()) {
			return false;
		}

		//若手中有道具，手中道具和触碰的道具一类，丢弃手中道具，不同类，手中道具放回背包
		if (CurrGripTarget != nullptr)
		{
			CurrGripTarget->ChangeData.RemoveDynamic(this, &AMotionControllerProp::ChangeData);

			EPropsType::Type GripPropType = CurrGripTarget->GetPropType();
			EPropsType::Type TouchingPropType = target->GetPropType();
			if (GripPropType == TouchingPropType) 
			{
				CurrGripTarget->ThrownAway();
			}
			else {
				CurrGripTarget->ThrownAway(true);
				CurrGripTarget->Destroy();
			}
		}

		UPrimitiveComponent* gripComp = target->GetGripComponent();
		if (gripComp)
		{
			FTransform gripTransform = target->GetGripTransform();
			GrabbedActorStartingInterpTransform = gripComp->GetComponentTransform();
			GripPhysicsHandle->GrabComponentAtLocationWithRotation(gripComp, TEXT("None"), GrabbedActorStartingInterpTransform.GetLocation(), GrabbedActorStartingInterpTransform.GetRotation().Rotator());
		}

		EPropsType::Type PropType = target->GetPropType();
		target->ChangeData.AddDynamic(this, &AMotionControllerProp::ChangeData);

		CurrGripTarget = target;
		ChangeData(CurrGripTarget);
		target->PickedUpStart(Instigator);

		IsGripping = true;

		return true;
		//TryPickActor(target);
	}

	return false;
}

AGraspedProp* AMotionControllerProp::GetGrabActor()
{
	return CurrGripTarget;
}

bool AMotionControllerProp::Grip1Pressed()
{
	Super::TriggerPressed();

	//TryGripTarget(TraceActor);
	ServerGripTarget(TraceActor);
	return true;
}

bool AMotionControllerProp::Grip1Released()
{
	Super::Grip1Released();
	if (IsPressing)
	{
		IsPressing = false;
		PressingTime = 0;
	}
	return true;
}

bool AMotionControllerProp::TriggerPressed()
{
	Super::TriggerPressed();

	/*if (CurrGripTarget)
	{
		bool bExhaust = false;
		if (bExhaust)
		{
			EPropsType::Type PropType = CurrGripTarget->GetPropType();
			Props.Remove(PropType);
			FollowMotion = false;
			CurrGripTarget->Destroy();
			CurrGripTarget = nullptr;
			ViveMesh->SetHiddenInGame(false, false);
			GripPhysicsHandle->ReleaseComponent();
		}
	}*/

	return true;
}

bool AMotionControllerProp::TriggerReleased()
{
	if (CurrGripTarget)
	{
	}
	return true;
}

void AMotionControllerProp::TryPickActor(AGraspedProp* target)
{
	if (target == nullptr ||
		!target->CanBePackup()) {
		return;
	}

	EPropsType::Type PropType = target->GetPropType();
	target->ChangeData.AddDynamic(this, &AMotionControllerProp::ChangeData);

	if (CurrGripTarget != nullptr)
	{
		CurrGripTarget->ChangeData.RemoveDynamic(this, &AMotionControllerProp::ChangeData);
		CurrGripTarget->Destroy();
		CurrGripTarget = nullptr;
	}

	CurrGripTarget = target;
	ChangeData(CurrGripTarget);
	target->PickedUpStart(Instigator);
	TraceActor = nullptr;

	ViveMesh->SetHiddenInGame(true, false);

	IsGripping = true;
}
/*
bool AMotionControllerProp::CalcGripTargetTransform_Validate(float fDelta)
{
	return true;
}*/

void AMotionControllerProp::CalcGripTargetTransform/*_Implementation*/(float fDelta)
{
	if (CurrGripTarget == nullptr) {
		return;
	}

	if (IsGripping)
	{
		FTransform newTransform = CalculateNewTransform(true);

		if (GripCurve)
		{
			float fMinTime, fMaxTime;
			GripCurve->GetTimeRange(fMinTime, fMaxTime);
			if (GripTime >= fMaxTime)
			{
				GripTime = 0.0f;
				IsGripping = false;
				FollowMotion = true;
				CurrGripTarget->PickedUpEnd();
			}
			else
			{
				float fGripValue = GripCurve->GetFloatValue(GripTime);
				FTransform targetTransform = CurrGripTarget->GetActorTransform();
				if (GripPhysicsHandle && GripPhysicsHandle->GetGrabbedComponent())
				{
					targetTransform = UKismetMathLibrary::TLerp(GrabbedActorStartingInterpTransform, newTransform, fGripValue);
					GripPhysicsHandle->SetTargetLocationAndRotation(targetTransform.GetLocation(), targetTransform.GetRotation().Rotator());
				}
				else {
					CurrGripTarget->SetActorTransform(UKismetMathLibrary::TLerp(CurrGripTarget->GetActorTransform(), newTransform, fGripValue));
				}

				GripTime += fDelta;
			}
		}
		else
		{
			CurrGripTarget->SetActorTransform(newTransform);
			GripTime = 0.0f;
			IsGripping = false;
			FollowMotion = true;

			CurrGripTarget->PickedUpEnd();
		}

	}
	else
	{
		if (FollowMotion)
		{
			FTransform newTransform = CalculateNewTransform(IsLaggyHand);
			AAbilityEntity* abilityEntity = Cast<AAbilityEntity>(CurrGripTarget);
			if (abilityEntity)
			{
				newTransform = abilityEntity->GetShakeOffset() * newTransform;
			}

			bool isPhysicsGrip = GripPhysicsHandle->GetGrabbedComponent() != nullptr;
			if (isPhysicsGrip)
			{
				GripPhysicsHandle->SetTargetLocationAndRotation(newTransform.GetLocation(), newTransform.GetRotation().Rotator());
			}
			else
			{
				CurrGripTarget->SetActorTransform(newTransform);
			}
			
		}
	}
}

FTransform AMotionControllerProp::CalculateNewTransform(bool disableLaggyHands)
{
	if (CurrGripTarget == nullptr) {
		return FTransform::Identity;
	}

	FVector motionLocation = ViveMesh->GetComponentLocation();
	FRotator motionRotator = ViveMesh->GetComponentRotation();

	FTransform targetTransform;
	if (disableLaggyHands)
	{
		float timeDilatedDeltaSeconds;
		timeDilatedDeltaSeconds = FMath::Clamp(UGameplayStatics::GetWorldDeltaSeconds(this), 0.0f, 0.044f) / UGameplayStatics::GetGlobalTimeDilation(this);

		FMass mass = CurrGripTarget->GetPropMass();
		FVector actorLocation = CurrGripTarget->GetActorLocation();
		FRotator actorRotator = CurrGripTarget->GetActorRotation();

		FVector laggyMassTarget = motionLocation + UKismetMathLibrary::GetForwardVector(motionRotator) * mass.LaggyLength;
		LaggyMassPosition = UKismetMathLibrary::VectorSpringInterp(LaggyMassPosition, laggyMassTarget, LaggySpringState, mass.ForwardStiffness, 1, timeDilatedDeltaSeconds, mass.Value);

		FVector upVectorTarget = UKismetMathLibrary::GetUpVector(motionRotator) * mass.LaggyLength;
		UpVectorSpring = UKismetMathLibrary::VectorSpringInterp(UpVectorSpring, upVectorTarget, UpVectorSpringState, mass.UpVectorStiffness, 1, timeDilatedDeltaSeconds, mass.Value);

		FVector laggyMassDiff = LaggyMassPosition - motionLocation;
		FRotator lookAtLaggyMassRotation = UKismetMathLibrary::MakeRotFromXZ(laggyMassDiff, UpVectorSpring);

		MotionPosition = UKismetMathLibrary::VectorSpringInterp(MotionPosition, motionLocation, MotionSpringState, mass.PositionStiffness, 1, timeDilatedDeltaSeconds, mass.Value);

		targetTransform = FTransform(lookAtLaggyMassRotation, MotionPosition, FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		targetTransform = FTransform(motionRotator, motionLocation, FVector(1.0f, 1.0f, 1.0f));
	}

	

	FTransform gripTransform = CurrGripTarget->GetGripTransform();
	targetTransform = gripTransform.Inverse() * targetTransform;
	targetTransform.SetScale3D(CurrGripTarget->GetActorScale());
	
	return targetTransform;
}

void AMotionControllerProp::MoveAimRotation(FVector targetLocation)
{
	if (CurrGripTarget)
	{
		//算出最终坐标
		FTransform gripTrans = CurrGripTarget->GetActorTransform();
		FRotator temp = FRotationMatrix::MakeFromX(targetLocation - gripTrans.GetLocation()).Rotator();
		gripTrans.SetRotation(FQuat(temp));
		
		//ViveMesh坐标
		FTransform motionTrans = CurrGripTarget->GetGripTransform() * gripTrans;

		//相对坐标
		FQuat actorToWorldQuat = GetActorQuat();
		const FQuat quat = actorToWorldQuat.Inverse() * motionTrans.GetRotation();

		GoldAimRotatior = quat.Rotator();
		IsMoveAimRotator = true;
		MoveAimTime = 0;
	}
}

void AMotionControllerProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMotionControllerProp, CurrGripTarget);
	DOREPLIFETIME(AMotionControllerProp, PropsInfo);

	DOREPLIFETIME_CONDITION(AMotionControllerProp, LaggyMassPosition, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, UpVectorSpring, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, MotionPosition, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, IsGripping, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, GripTime, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, FollowMotion, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AMotionControllerProp, GrabbedActorStartingInterpTransform, COND_InitialOnly);
}

void AMotionControllerProp::ChangeProp(EPropsType::Type type)
{
	FGraspedPropInfo PropInfo = PropsInfo[type];
	if (PropInfo.PropClass == nullptr) {
		return;
	}

	if (CurrGripTarget && type == CurrGripTarget->GetPropType()) {
		return;
	}

	if (CurrGripTarget)
	{
		CurrGripTarget->ChangeData.RemoveDynamic(this, &AMotionControllerProp::ChangeData);
		CurrGripTarget->SetActorHiddenInGame(true);
		CurrGripTarget->ThrownAway(true);
		CurrGripTarget->Destroy();
		CurrGripTarget = nullptr;
	}

	CurrGripTarget = GetWorld()->SpawnActor<AGraspedProp>(PropInfo.PropClass, FVector::ZeroVector, FRotator::ZeroRotator);
	CurrGripTarget->SetActorTransform(CalculateNewTransform(false));
	CurrGripTarget->ChangeData.AddDynamic(this, &AMotionControllerProp::ChangeData);
	CurrGripTarget->SetCurData(PropInfo.Data1, PropInfo.Data2);
	CurrGripTarget->PickedUpStart(Instigator);
	CurrGripTarget->PickedUpEnd();

	GripTime = 0.0f;
	IsGripping = false;
	FollowMotion = true;
}

bool AMotionControllerProp::SeverChangeProp_Validate(EPropsType::Type type)
{
	return true;
}

void AMotionControllerProp::SeverChangeProp_Implementation(EPropsType::Type type)
{
	FGraspedPropInfo PropInfo = PropsInfo[type];
	if (PropInfo.PropClass == nullptr) {
		return;
	}

	if (CurrGripTarget && type == CurrGripTarget->GetPropType()) {
		return;
	}

	if (CurrGripTarget)
	{
		CurrGripTarget->ChangeData.RemoveDynamic(this, &AMotionControllerProp::ChangeData);
		CurrGripTarget->SetActorHiddenInGame(true);
		CurrGripTarget->ThrownAway();
		CurrGripTarget->Destroy();
		CurrGripTarget = nullptr;
	}

	CurrGripTarget = GetWorld()->SpawnActor<AGraspedProp>(PropInfo.PropClass, FVector::ZeroVector, FRotator::ZeroRotator);
	CurrGripTarget->SetActorTransform(CalculateNewTransform(false));
	CurrGripTarget->ChangeData.AddDynamic(this, &AMotionControllerProp::ChangeData);
	CurrGripTarget->SetCurData(PropInfo.Data1, PropInfo.Data2);
	CurrGripTarget->PickedUpStart(Instigator);
	CurrGripTarget->PickedUpEnd();

	GripTime = 0.0f;
	IsGripping = false;
	FollowMotion = true;
}

void AMotionControllerProp::ChangeData(AGraspedProp* prop)
{
	if (prop == CurrGripTarget)
	{
		EPropsType::Type propType = prop->GetPropType();
		FGraspedPropInfo info;
		info.PropClass = prop->GetClass();

		AAbilityEntity* entity = Cast<AAbilityEntity>(prop);
		if (entity)
		{
			info.EntityParamID = entity->GetParamID();
			info.Data1 = entity->GetCurNumInGroup();
			info.Data2 = entity->GetCurNumInpak();
		}
		PropsInfo[int(propType)] = info;
	}
}
