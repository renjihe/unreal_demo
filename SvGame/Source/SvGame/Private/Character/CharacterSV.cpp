// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "CharacterSV.h"
#include "LookRes.h"
#include "DismemberActor.h"
#include "PlayerStateSV.h"
#include "MajorCharacter.h"
#include "Abilities/GameplayAbilitySV.h"
#include "Abilities/HitTestActor.h"
#include "Res/ParticleRes.h"
#include "Res/SoundRes.h"
//#include "Res/ImpactEffectRes.h"
#include "Effect/HitImpactEffect.h"

#include "Kismet/KismetMathLibrary.h"
#include "Components/MeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Task/AbilityTask_PlayMontageAndWaitSV.h"

#define FLOATZERO 0.00001f

#define PHYSICS_BLEND_TIME	0.5f
FRichCurve gPhysicsBlendCurve;
static int sSerialNumber = 0;

// Sets default values
ACharacterSV::ACharacterSV(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCharacterSVMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);

	GetMesh()->SetCollisionProfileName(FName("PhysicsActor"));
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->bGenerateOverlapEvents = true;

	GetMesh()->SetConstraintProfileForAll(FName("Hittable"));
	//GetMesh()->SetEnablePhysicsBlending(true);

	PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimation"));
	PhysicalAnimation->SetSkeletalMeshComponent(GetMesh());
	FPhysicalAnimationData physAnimData;
	physAnimData.OrientationStrength = 1000;
	physAnimData.AngularVelocityStrength = 100;
	physAnimData.PositionStrength = 1000;
	physAnimData.VelocityStrength = 100;
	PhysicalAnimation->ApplyPhysicalAnimationSettings(NAME_None, physAnimData);
	PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, FName("Hittable"));

	// -------------------------------------------------------------------------------------------
	//			GameplayAbility System Sample
	// -------------------------------------------------------------------------------------------
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComp->SetupAttachment(RootComponent);

	//初始化Ability
	AbilityList.Reset();
	AbilityStateList.Reset();
	AbilityBarState.Reset();
	for (int i = 0; i < EAbilityBar::Count; ++i)
	{
		AbilityList.Add(INVALID_ABILITY);
		AbilityBarState.Add(false);
	}

	TeamID = NoTeamId;
	CurrLookId = 0;
	LookId = Invalid_Look;
	OrginLookId = Invalid_Look;
	CharRole = ECharacterRole::MINION;
	bDead = false;
	DisableImpulse = false;
	DisableDismember = false;
	bDirtyClientBuff = bDirtyClientDebuff = false;

	bRagdoll = false;
	bFallingCollisionMode = false;
	bNoCollisionMode = false;
	bInSafeHouse = false;

	MoveSpeedTarget = MoveSpeedCurrent = 0;
	SpeedType = ESpeedType::Walk;
	
	if (gPhysicsBlendCurve.GetNumKeys() == 0)
	{
		gPhysicsBlendCurve.AddKey(0, 0.1f);
		gPhysicsBlendCurve.AddKey(0.1f, 0.6f);
		gPhysicsBlendCurve.AddKey(0.2f, 0.5f);
		gPhysicsBlendCurve.AddKey(0.3f, 0.2f);
		gPhysicsBlendCurve.AddKey(0.4f, 0.1f);
		gPhysicsBlendCurve.AddKey(0.5f, 0.0f);
	}

	//Spline
	bTraceSpline = false;
	SplineMeshs.Reset();
}

// Called when the game starts or when spawned
void ACharacterSV::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterSV::Init(FVector Location, FRotator Rotator)
{
	OrginLocation = Location;
	OrginRotator = Rotator;

	if (Invalid_Look == OrginLookId) {
		OrginLookId = LookId;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (Role == ROLE_Authority) {
			InitAbility();
		}

		AbilitySystemComponent->AbilityActivatedCallbacks.AddStatic(&ACharacterSV::AbilityActivatedCallBack);
		AbilitySystemComponent->AbilityEndedCallbacks.AddStatic(&ACharacterSV::AbilityEndedCallback);

		UE_LOG(LogTemp, Display, TEXT("%s BeginPlay: %s"), *GetFullName(), *GetNameSafe(AbilitySystemComponent->AbilityActorInfo->PlayerController.Get()));
	}
}

// Called every frame
void ACharacterSV::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority) 
	{
		UpdateAbility(DeltaTime);
		TickBuff();
	}
	else
	{
		do
		{
			if (IsMajor())
			{
				AMajorCharacter *pawn = Cast<AMajorCharacter>(this);
				if (pawn && (NULL == pawn->MotionController || NULL == pawn->MotionController->GetGrabActor()))
					break;
			}

			if (bDirtyClientDebuff)
			{
				OnRep_DoBuff(true);
				bDirtyClientDebuff = false;
			}

			if (bDirtyClientBuff)
			{
				OnRep_DoBuff(false);
				bDirtyClientBuff = false;
			}
		} while (0);
	}
	
	UpdateLook();

	TickPhysics(DeltaTime);

	TickMoveSpeed(DeltaTime);

	if (IsLocallyControlled() && bTraceSpline) {
		UpdateSplinePath();
	}

	if (CharRole == ECharacterRole::MOB && GetMovementComponent()->IsFalling() != bFallingCollisionMode)
	{
		bFallingCollisionMode = !bFallingCollisionMode;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, bFallingCollisionMode ? ECR_Overlap : ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel9, bFallingCollisionMode ? ECR_Overlap : ECR_Block);
	}

	/*if (bDead && IsMob())
	{
		if (Controller) {
			Controller->
		}
		//SetActorTransform(GetMesh()->GetComponentTransform());
	}*/
}

void ACharacterSV::TickMoveSpeed(float deltaTime)
{
	if (!IsMajor())
	{
		if (Role == ROLE_Authority && Controller)
		{
			AAIController *ai = Cast<AAIController>(Controller);
			if (ai) {
				MoveSpeedTarget = ai->GetMoveStatus() == EPathFollowingStatus::Moving ? CalcSpeedAttrib() : 0;
			}
		}

		if (MoveSpeedCurrent != MoveSpeedTarget) {
			MoveSpeedCurrent = UKismetMathLibrary::FInterpTo(MoveSpeedCurrent, MoveSpeedTarget, deltaTime, 1.0f);
		}

		if (!FMath::IsNearlyZero(MoveSpeedCurrent))
		{
			float fwd_speed = 0;
			if (!GetMesh()->GetAnimInstance()->GetCurveValue("Fwd Vel", fwd_speed)) {
				//fwd_speed = 600.0f;
			}

			((UCharacterMovementComponent*)GetMovementComponent())->MaxWalkSpeed = fwd_speed;

			float lateral_dist = 0;
			if (!GetMesh()->GetAnimInstance()->GetCurveValue("Lateral Dist", lateral_dist)) {
				lateral_dist = 0;
			}

			GetMesh()->RelativeLocation.Y = -lateral_dist;
		}
	}	
}

/*FVector ACharacterSV::GetMoveGoalOffset(const AActor* MovingActor) const
{
	const AAIController *controller = Cast<AAIController>(MovingActor);
	if (controller)
	{
		APawn *pawn = controller->GetPawn();
		if (pawn)
		{
			FVector direction = pawn->GetActorLocation() - GetActorLocation();
			float radius = direction.Size() > 500 ? 400 : 0;

			if (radius)
			{
				UNavigationSystem* navSys = UNavigationSystem::GetCurrent(GetWorld());
				if (navSys)
				{
					ANavigationData* navData = navSys->GetNavDataForProps(controller->GetNavAgentPropertiesRef());
					if (navData == nullptr) {
						navData = navSys->GetMainNavData(FNavigationSystem::DontCreate);
					}

					FNavLocation outResult;
					if (navSys->GetRandomReachablePointInRadius(GetActorLocation() + direction.GetSafeNormal() * radius, radius, outResult, navData, UNavigationQueryFilter::GetQueryFilter(*navData, GetWorld(), controller->GetDefaultNavigationFilterClass()))) {
						return outResult.Location - GetActorLocation();
					}
				}
			}
		}
	}

	return FVector::ZeroVector;
}*/

#if WITH_EDITOR
void ACharacterSV::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateLook();
}
#endif

void ACharacterSV::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UpdateLook();
}

// Called to bind functionality to input
void ACharacterSV::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterSV::PossessedBy(AController *newController)
{
	Super::PossessedBy(newController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		UE_LOG(LogTemp, Display, TEXT("PossessedBy (%s): %s"), *GetNameSafe(newController), *GetNameSafe(AbilitySystemComponent->AbilityActorInfo->PlayerController.Get()));
	}

	APlayerStateSV* ps = Cast<APlayerStateSV>(PlayerState);
	if (ps && !ps->bInitAttrib && JobId != Invalid_Job)
	{
		FJobRow job = FJobRes::Get().GetJob(JobId);
		for (auto &elem : job.Attributes)
		{
			SetAttrib(elem.Key, elem.Value);
		}
		if (GetAttrib(EAttribType::Health) == -1) {
			SetAttrib(EAttribType::Health, GetAttrib(EAttribType::MaxHealth));
		}
		GetCharacterMovement()->MaxWalkSpeed = GetAttrib(EAttribType::Speed);

		ps->bInitAttrib = true;
	}
}

void ACharacterSV::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Our PlayerController changed, must update ActorInfo on AbilitySystemComponent
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RefreshAbilityActorInfo();

		UE_LOG(LogTemp, Display, TEXT("%s OnRep_Controller (%s): %s"), *GetFullName(), *GetNameSafe(Controller), *GetNameSafe(AbilitySystemComponent->AbilityActorInfo->PlayerController.Get()));
	}
}

void ACharacterSV::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterSV, LookId);
	DOREPLIFETIME(ACharacterSV, JobId);

	DOREPLIFETIME(ACharacterSV, SpeedType);
	DOREPLIFETIME(ACharacterSV, MoveSpeedTarget);

	DOREPLIFETIME(ACharacterSV, AbilityList);
	DOREPLIFETIME(ACharacterSV, AbilityStateList);
	DOREPLIFETIME(ACharacterSV, AbilityBarState);

	DOREPLIFETIME(ACharacterSV, BuffData);
	DOREPLIFETIME(ACharacterSV, DebuffData);
}

void ACharacterSV::UpdateLook()
{
	if (CurrLookId != LookId) {
		SetLook(LookId);
	}
	//APlayerStateSV *state = Cast<APlayerStateSV>(PlayerState);
	//if (state == nullptr) {
	//	return;
	//}

	//// 3P Mesh
	//if (state->LookId != 0 && GetMesh()) {
	//	FLookRes::Get().CreateLook(state->LookId, GetMesh());
	//}

	// TODO: 1P Mesh
}

void ACharacterSV::SetLook(int lookId)
{
	CurrLookId = LookId = lookId;
	if (LookId == Invalid_Look) {
		return;
	}
	FLookRes::Get().CreateLook(lookId, GetMesh());

	if (GetMesh()) {
		GetMesh()->RelativeLocation.Z = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	}
}

void ACharacterSV::SetDead(bool Dead)
{ 
	bDead = Dead;
	OnCharacterDead.Broadcast(this);
}

UMeshComponent* ACharacterSV::GetSpecMesh(ESpecMesh::Type type)
{
	switch (type)
	{
	case ESpecMesh::Self:
		return GetMesh();
	default:
		return nullptr;
	}

	return nullptr;
}

AActor* ACharacterSV::GetSpecActor(ESpecMesh::Type type)
{
	return ESpecMesh::Self == type ? this : nullptr;
}

UMeshComponent* ACharacterSV::GetSpecMeshByPosition(EEffectPosition::Type position)
{
	UMeshComponent *ret = nullptr;
	switch (position)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::Actor:
	case EEffectPosition::MeshAttached:
	case EEffectPosition::ActorAttached:
		ret = GetSpecMesh(ESpecMesh::Self);
		break;
	case EEffectPosition::GripMesh:
	case EEffectPosition::GripActor:
	case EEffectPosition::GripMeshAttached:
	case EEffectPosition::GripActorAttached:
		ret = GetSpecMesh(ESpecMesh::GripActor);
		break;
	default:
		ret = nullptr;
		break;
	}

	return ret;
}

int ACharacterSV::CalcSpeedAttrib()
{
	float speedTypeGain = 1.0f;
	const FJobRow &jobInfo = FJobRes::Get().GetJob(JobId);
	const float *speedTypeGainPtr = jobInfo.SpeedTypeGain.Find(SpeedType);
	if (speedTypeGainPtr) {
		speedTypeGain = *speedTypeGainPtr;
	}
	else 
	{
		const FGlobalConfigRow *config = &FGlobalConfigRes::Get().GetConfig("SpeedTypeGain");
		if (config && config->FloatValue.Num() >= ESpeedType::Count) {
			speedTypeGain = config->FloatValue[SpeedType];
		}
	}

	return GetAttrib(EAttribType::Speed) + speedTypeGain + GetAttrib(EAttribType::GainSpeed);
}

void ACharacterSV::SetSpeedType(ESpeedType::Type Type)
{
	SpeedType = Type;
	//GetCharacterMovement()->MaxWalkSpeed = CalcSpeedAttrib();
}

//Team
//////////////////////////////////////////////////////////////////////////
int ACharacterSV::GetTeamID()
{
	return TeamID;
}

void ACharacterSV::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID.GetId();
}

FGenericTeamId ACharacterSV::GetGenericTeamId() const
{
	return FGenericTeamId(TeamID);
}

ETeamAttitude::Type ACharacterSV::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent && GetGenericTeamId() != NoTeamId && OtherTeamAgent->GetGenericTeamId() != NoTeamId)
	{
		if (GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId()) {
			return ETeamAttitude::Friendly;
		}
		else {
			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Neutral;
}

ETeamAttitude::Type ACharacterSV::GetTeamAttitudeTowards(AActor* target1, AActor* target2)
{
	const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(target1);
	if (TeamAgent)
	{
		TeamAgent->GetTeamAttitudeTowards(*target2);
	}

	return ETeamAttitude::Neutral;
}

ETeamAttitude::Type ACharacterSV::GetTeamAttitudeTowardsByTeamID(int targetTeamID, AActor* target)
{
	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(target);
	if (TeamAgent && targetTeamID != NoTeamId && TeamAgent->GetGenericTeamId() != NoTeamId)
	{
		if (targetTeamID == TeamAgent->GetGenericTeamId()) {
			return ETeamAttitude::Friendly;
		}
		else {
			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Neutral;
}

//Buff
void ACharacterSV::TickBuff()
{
	bool bCanRemove[2] = {false, false};
	for (unsigned j = 0; j < 2; ++j)
	{
		TArray<FBuff> &buffData = j == 0 ? BuffData : DebuffData;
		for (int i = 0; i < buffData.Num(); ++i)
		{
			FBuff &bf = buffData[i];
			if (NULL == bf.Info || 0 == bf.FrameCounter) 
			{
				bCanRemove[j] = true;
				continue;
			}

			if (--bf.FrameCounter == 0) 
			{
				ActiveBuff(bf, false);
				continue;
			}

			if (bf.BuffPeriodicFrames && ++bf.PeriodicCounter >= bf.BuffPeriodicFrames)
			{
				bf.PeriodicCounter = 0;

				for (int k = 0; k < bf.Info->Funcs.Num(); ++k)
				{
					if (bf.Info->Funcs[k].Type == EBufFuncType::PERIODIC) {
						DoBuff(&FBufRes::Get().GetBuf(bf.Info->Funcs[k].Args[0]), true);
					}
				}
			}
		}
	}
	
	for (unsigned j = 0; j < 2; ++j)
	{
		if (bCanRemove[j])
		{
			TArray<FBuff> &buffData = j == 0 ? BuffData : DebuffData;
			buffData.RemoveAllSwap([](FBuff Buff) {
				return NULL == Buff.Info || 0 == Buff.FrameCounter;
			});
		}
	}
}

void ACharacterSV::AddBuff(int BuffID, int Frames, int BuffPeriodicFrames, FAttribs CasterAttrib)
{
	const FBufRow *Info = &FBufRes::Get().GetBuf(BuffID);
	if (Info)
	{
		if (Info->bOverride)
		{
			FBuff *buff = FindBuffByID(BuffID);
			if (buff) 
			{
				buff->FrameCounter = Frames;
				buff->CasterAttrib = CasterAttrib;
				return;
			}
		}

		FBuff bf;
		bf.Info = Info;
		bf.BuffID = BuffID;
		bf.SerialNumber = ++sSerialNumber;
		bf.FrameCounter = Frames;
		bf.EffectID = 0;
		memset(bf.UserData, 0, sizeof(bf.UserData));
		bf.PeriodicCounter = 0;
		bf.BuffPeriodicFrames = BuffPeriodicFrames;
		bf.CasterAttrib = CasterAttrib;

		TArray<FBuff> &buffData = Info->bDebuf ? DebuffData : BuffData;
		int index = buffData.Add(bf);

		ActiveBuff(buffData[index], true);
	}
}

void ACharacterSV::RemoveBuff(int BuffID)
{
	for (unsigned j = 0; j < 2; ++j)
	{
		TArray<FBuff> &buffData = j == 0 ? BuffData : DebuffData;
		for (int i = 0; i < buffData.Num(); ++i)
		{
			FBuff &bf = buffData[i];
			if (NULL == bf.Info || 0 == bf.FrameCounter) {
				continue;
			}

			if (bf.BuffID == BuffID) {
				ActiveBuff(bf, false);
			}
		}
	}
}

void ACharacterSV::ActiveBuff(FBuff &Buff, bool bActive, bool bClent)
{
	DoBuff(Buff.Info, bActive, &Buff, bClent);

	if (!bActive)
	{
		Buff.Info = NULL;
		Buff.FrameCounter = 0;
	}
}

void ACharacterSV::DoBuff(const FBufRow *Info, bool bActive, FBuff *Buff, bool bClent)
{
	if (NULL == Info)
		return;

	if (bActive)
	{
		for (int i = 0; i < Info->Funcs.Num(); ++i)
		{
			if (Info->Funcs[i].Type == EBufFuncType::NONE) {
				continue;
			}

			if (!bClent && Info->Funcs[i].Type == EBufFuncType::ATTRIB)
			{
				if (Info->Funcs[i].Args.Num() < 2)
					continue;

				EAttribType::Type type = (EAttribType::Type)Info->Funcs[i].Args[0];
				if (type < EAttribType::Count) {
					AddAttrib(type, Info->Funcs[i].Args[1]);
				}
			}
			else if (!bClent && Info->Funcs[i].Type == EBufFuncType::COSPLAY)
			{
				if (Info->Funcs[i].Args.Num() < 1)
					continue;

				SetLook(Info->Funcs[i].Args[0]);
			}
			else if (Info->Funcs[i].Type == EBufFuncType::EFFECT) 
			{
				if (Buff)
				{
					if (Info->Funcs[i].Args.Num() < 3)
						continue;
					if (Info->Funcs[i].StringArgs.Num() < 1)
						continue;
					if (Info->Funcs[i].VectorArgs.Num() < 2)
						continue;

					UParticleSystemComponent* PSC = PlayParticleInternal(Info->Funcs[i].Args[0], this
						, (EEffectPosition::Type)Info->Funcs[i].Args[1], Info->Funcs[i].StringArgs[0]
						, Info->Funcs[i].VectorArgs[0], FRotator(Info->Funcs[i].VectorArgs[1].X, Info->Funcs[i].VectorArgs[1].Y, Info->Funcs[i].VectorArgs[1].Z), (1 == Info->Funcs[i].Args[2]));

					if (PSC && 0 == Info->Funcs[i].Args[2]) {
						Buff->UserDataPtr[i] = PSC;
					}
				}
			}
			else if (Info->Funcs[i].Type == EBufFuncType::EXTREMIS)
			{
				//SetDead(true);
				NetMulticastNotifyDead(FHitResult());
			}
		}
	}
	else
	{
		for (int i = 0; i < Info->Funcs.Num(); ++i)
		{
			if (Info->Funcs[i].Type == EBufFuncType::NONE) {
				continue;
			}

			if (!bClent && Info->Funcs[i].Type == EBufFuncType::ATTRIB)
			{
				if (Info->Funcs[i].Args.Num() < 2)
					continue;

				EAttribType::Type type = (EAttribType::Type)Info->Funcs[i].Args[0];
				if (type < EAttribType::Count) {
					AddAttrib(type, -(Info->Funcs[i].Args[1]));
				}
			}
			else if (!bClent && Info->Funcs[i].Type == EBufFuncType::COSPLAY) {
				SetLook(OrginLookId);
			}
			else if (Info->Funcs[i].Type == EBufFuncType::EFFECT)
			{
				if (Buff)
				{
					switch (Info->Funcs[i].Args[1])
					{
					case EEffectPosition::MeshAttached:
					case EEffectPosition::GripMeshAttached:
					case EEffectPosition::ActorAttached:
					case EEffectPosition::GripActorAttached:
						{
							UParticleSystemComponent* PSC = (UParticleSystemComponent*)Buff->UserDataPtr[i];
							if (PSC) 
							{
								PSC->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
								PSC->DestroyComponent();
							}
						}
						break;
					default:
						break;
					}
					Buff->UserDataPtr[i] = NULL;
				}
			}
		}
	}
}

FBuff *ACharacterSV::FindBuffByID(int BuffID, bool bClient)
{
	for (unsigned j = 0; j < 2; ++j)
	{
		TArray<FBuff> &buffData = bClient ? (j == 0 ? ClientDebuffData : ClientBuffData)
											: (j == 0 ? DebuffData : BuffData);

		for (int i = 0; i < buffData.Num(); ++i)
		{
			FBuff &bf = buffData[i];
			if (NULL == bf.Info || 0 == bf.FrameCounter) {
				continue;
			}

			if (bf.BuffID == BuffID) {
				return &bf;
			}
		}
	}
	return NULL;
}

FBuff *ACharacterSV::FindBuffBySerial(int SerialNumber, bool bClient)
{
	for (unsigned j = 0; j < 2; ++j)
	{
		TArray<FBuff> &buffData = bClient ? (j == 0 ? ClientDebuffData : ClientBuffData)
			: (j == 0 ? DebuffData : BuffData);

		for (int i = 0; i < buffData.Num(); ++i)
		{
			FBuff &bf = buffData[i];
			if (0 == bf.FrameCounter) {
				continue;
			}

			if (bf.SerialNumber == SerialNumber) {
				return &bf;
			}
		}
	}
	return NULL;
}

void ACharacterSV::OnRep_DebuffData()
{
	bDirtyClientDebuff = true;
	//OnRep_DoBuff(true);
}

void ACharacterSV::OnRep_BuffData()
{
	bDirtyClientBuff = true;
	//OnRep_DoBuff(false);
}

void ACharacterSV::OnRep_DoBuff(bool bDebuf)
{
	if (Role == ROLE_Authority)
		return;

	TArray<FBuff> &buffData = bDebuf ? DebuffData : BuffData;
	TArray<FBuff> &clientBuffData = bDebuf ? ClientDebuffData : ClientBuffData;

	for (int i = 0; i < buffData.Num(); ++i)
	{
		FBuff *buff = FindBuffBySerial(buffData[i].SerialNumber, true);
		if (NULL == buff)
		{
			FBuff newBuff;
			newBuff = buffData[i];
			newBuff.Info = &FBufRes::Get().GetBuf(buffData[i].BuffID);
			int index = clientBuffData.Add(newBuff);

			ActiveBuff(clientBuffData[index], true, true);
		}
	}

	bool bCanRemove = false;
	for (int i = 0; i < clientBuffData.Num(); ++i)
	{
		FBuff *buff = FindBuffBySerial(clientBuffData[i].SerialNumber, false);
		if (NULL == buff) 
		{
			bCanRemove = true;
			ActiveBuff(clientBuffData[i], false, true);
		}
	}

	if (bCanRemove)
	{
		clientBuffData.RemoveAllSwap([](FBuff Buff) {
			return NULL == Buff.Info || 0 == Buff.FrameCounter;
		});
	}
}

//Attrib
//////////////////////////////////////////////////////////////////////////
int ACharacterSV::GetAttrib(EAttribType::Type attribType)
{
	APlayerStateSV *ps = (APlayerStateSV*)PlayerState;
	if (ps && attribType < EAttribType::Count) {
		return ps->Attrib[attribType];
	}
	return 0;
}

FAttribs ACharacterSV::GetAttribs()
{
	APlayerStateSV *ps = (APlayerStateSV*)PlayerState;
	if (ps) {
		return ps->Attrib;
	}
	return FAttribs();
}

void ACharacterSV::SetAttrib(EAttribType::Type attribType, int value)
{
	APlayerStateSV *ps = (APlayerStateSV*)PlayerState;
	if (ps && attribType < EAttribType::Count)
	{
		ps->Attrib[attribType] = value;
		if (attribType == EAttribType::Speed)
		{
			GetCharacterMovement()->MaxWalkSpeed = value;
		}
	}
}

void ACharacterSV::AddAttrib(EAttribType::Type attribType, int value)
{
	APlayerStateSV *ps = (APlayerStateSV*)PlayerState;
	if (ps && attribType < EAttribType::Count) {
		ps->Attrib[attribType] += value;
	}
}

CharacterBodyPart::Type ACharacterSV::GetBodyPart(EPhysicalSurface Surface)
{
	switch (Surface)
	{
	case SurfaceType_Default:
		return CharacterBodyPart::Body;
	case SurfaceType2:
		return CharacterBodyPart::Head;
	case SurfaceType3:
		return CharacterBodyPart::Body;
	case SurfaceType4:
		return CharacterBodyPart::ArmL;
	case SurfaceType5:
		return CharacterBodyPart::ArmR;
	case SurfaceType6:
		return CharacterBodyPart::LegL;
	case SurfaceType7:
		return CharacterBodyPart::LegR;
	default:
		return CharacterBodyPart::Body;
	}
}

float ACharacterSV::CalcProjectileDamage(float damage, FAttribs attackAttrib, FAttribs targetAttrib, AAbilityEntity* entity, EPhysicalSurface surface, float distance)
{
	if (entity)
	{
		FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(entity->GetParamID());
		float partAdjust = 1.0f;
		if (param.PartAdjust.Find(surface)) {
			partAdjust = param.PartAdjust[surface];
		}

		float partArmor = 1.0f * (targetAttrib[EAttribType::ArmorProjectile] / 100.0f);
		const FGlobalConfigRow *config = &FGlobalConfigRes::Get().GetConfig("PartArmor");
		if (surface > SurfaceType1 && surface - SurfaceType2 < config->FloatValue.Num()) {
			partArmor *= config->FloatValue[surface - SurfaceType2];
		}

		float dampPow = FMath::Max(distance - param.DampRadius, 0.0f) / param.DampRadius;
		damage = damage * partAdjust * partArmor * FMath::Pow(param.Damp, dampPow);
	}
	return damage;
}

float ACharacterSV::CalcMeleeDamage(float damage, FAttribs attackAttrib, FAttribs targetAttrib)
{
	float partArmor = 1.0f * (targetAttrib[EAttribType::ArmorProjectile] / 100.0f);
	return damage * partArmor;
}

//Ability
//////////////////////////////////////////////////////////////////////////
bool ACharacterSV::IsExistAbility(int abilityID)
{
	for (int i = 0; i < AbilityList.Num(); ++i)
	{
		if (AbilityList[i] == abilityID) {
			return true;
		}
	}

	return false;
}

int ACharacterSV::GetAbilityIndex(int abilityID)
{
	for (int i = 0; i < AbilityList.Num(); ++i)
	{
		if (AbilityList[i] == abilityID) {
			return i;
		}
	}

	return EAbilityBar::Count;
}

FAbilityInfoRow ACharacterSV::GetAbilityInfoByID(int abilityID)
{
	for (int i = 0; i < AbilityList.Num(); ++i)
	{
		if (AbilityList[i] == abilityID) {
			return AbilityInfoRes::Get().GetAbilityinfo(abilityID);;
		}
	}

	return AbilityInfoRes::DefaultRow;
}

FAbilityInfoRow ACharacterSV::GetAbilityInfoByIndex(int index) const
{
	if (index >= 0 && index < AbilityList.Num()) {
		return AbilityInfoRes::Get().GetAbilityinfo(AbilityList[index]);
	}
	return AbilityInfoRes::DefaultRow;
}

int ACharacterSV::GetAbilityIDByIndex(EAbilityBar::Type index) const
{
	if (index >= 0 && index < AbilityList.Num()) {
		return AbilityList[index];
	}
	return 0;
}

int ACharacterSV::SelectAttackAbility() const
{
	int abilityID = 0;
	float attackRadius = 0.0f;
	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	for (int i = 0; i < AbilityList.Num(); ++i)
	{
		FAbilityState abilityState = GetAbilityStateByIndex(AbilityList[i]);
		if (abilityState.CD > 0.0f) {
			continue;
		}
		FAbilityInfoRow abilityInfo = abilityInfoRes.GetAbilityinfo(AbilityList[i]);

		if (abilityInfo.AbilityFlags.IsAttack() && (abilityID == 0 || attackRadius < abilityInfo.Radius))
		{
			abilityID = AbilityList[i];
			attackRadius = abilityInfo.Radius;
		}

	}

	return abilityID;
}

int ACharacterSV::SelectCureAbility() const
{
	int abilityID = 0;
	int value = 0;
	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	FAbilityInfoRow ret = abilityInfoRes.DefaultRow;

	for (int i = 0; i < AbilityList.Num(); ++i)
	{
		FAbilityState abilityState = GetAbilityStateByIndex(AbilityList[i]);
		if (abilityState.CD > 0.0f) {
			continue;
		}
		FAbilityInfoRow abilityInfo = abilityInfoRes.GetAbilityinfo(AbilityList[i]);

		if (abilityInfo.AbilityFlags.IsCure() && (abilityID == 0 || value < abilityInfo.Value))
		{
			abilityID = AbilityList[i];
			value = abilityInfo.Value;
		}
	}

	return abilityID;
}

int ACharacterSV::GetAbilityStateIndex(int abilityID)
{
	for (int i = 0; i < AbilityStateList.Num(); ++i)
	{
		if (AbilityStateList[i].AbilityID == abilityID)
		{
			return i;
		}
	}
	return -1;
}

FAbilityState ACharacterSV::GetAbilityStateByID(int abilityID)
{
	for (int i = 0; i < AbilityStateList.Num(); ++i)
	{
		if (AbilityStateList[i].AbilityID == abilityID)
		{
			return AbilityStateList[i];
		}
	}

	return FAbilityState();
}

FAbilityState ACharacterSV::GetAbilityStateByIndex(int Index) const
{
	if (Index >= 0 && Index < AbilityStateList.Num()) {
		return AbilityStateList[Index];
	}

	return FAbilityState();
}

bool ACharacterSV::IsUsedAbility()
{
	for (int i = 0; i < AbilityStateList.Num(); ++i) 
	{
		FAbilityState abilisyState = AbilityStateList[i];
		if (abilisyState.IsRunning && IsExistAbility(abilisyState.AbilityID)) {
			return true;
		}
	}
	return false;
}

bool ACharacterSV::IsIndexAbilityUsing(EAbilityBar::Type type)
{
	if (type > EAbilityBar::Count) {
		return false;
	}

	FAbilityState abilisyState = GetAbilityStateByID(AbilityList[type]);
	return IsExistAbility(abilisyState.AbilityID) && abilisyState.IsRunning;
}

void ACharacterSV::InitAbility()
{
	if (JobId == Invalid_Job) {
		return;
	}
	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	FJobRow jobInfo = FJobRes::Get().GetJob(JobId);
	
	for (auto &elem : jobInfo.AbilityList)
	{
		//int AbilityID = elem.Value;
		//此技能存在并且角色此技能栏没有技能
		if (elem.Key >= 0 && elem.Key < AbilityList.Num() && abilityInfoRes.IsExist(elem.Value) && AbilityList[elem.Key] == INVALID_ABILITY)
		{
			int abilityID = elem.Value;
			AbilityList[elem.Key] = abilityID;

			FAbilityInfoRow &abilityInfo = abilityInfoRes.GetAbilityinfo(abilityID);
			FAbilityState abilityState;
			abilityState.AbilityID = abilityID;
			FGameplayAbilitySpec spec(abilityInfo.GameAbility->GetDefaultObject<UGameplayAbilitySV>(), 1, (int32)elem.Key);
			abilityState.GameplayAbilitySpec = AbilitySystemComponent->GiveAbility(spec);

			//初始化Ability
			UGameplayAbility* instancedAbility = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec)->GetPrimaryInstance();
			if (instancedAbility == nullptr) {
				instancedAbility = spec.Ability;
			}
			UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility);
			if (abilitySV)
			{
				abilitySV->SetAbilityInfo(abilityID, abilityInfo, GetAttribs(), TeamID, elem.Key);
				abilitySV->InitAbility();
			}

			
			AbilityStateList.Add(abilityState);
		}
	}
}

void ACharacterSV::UpdateAbility(float DeltaTime)
{
	AbilityInfoRes abilityRes = AbilityInfoRes::Get();

	for (auto &elem : AbilityStateList)
	{
		FAbilityState& abilityState = elem;

		if (Role == ROLE_Authority)
		{
			FGameplayAbilitySpec* spec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec);
			UGameplayAbility* instancedAbility = spec->GetPrimaryInstance();
			if (instancedAbility == nullptr) {
				instancedAbility = spec->Ability;
			}
			if (UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility))
			{
				if (abilitySV->IsNeedTick()) {
					abilitySV->AbilityTick(DeltaTime);
				}
			}
		}
		
		if (abilityState.IsRunning)
		{
			abilityState.ActiveTime += DeltaTime;
		}
		else
		{
			if (abilityState.CD > 0.0f)
			{
				abilityState.CD -= DeltaTime;
				FAbilityInfoRow abilityInfo = abilityRes.GetAbilityinfo(abilityState.AbilityID);

				if (abilityState.CD <= 0.0f)
				{
					abilityState.CD = 0.0f;
					if (AbilityBarState[GetAbilityIndex(abilityState.AbilityID)] && abilityInfo.Loop) {
						TryActivateAbilityByID(abilityState.AbilityID);
					}
				}
			}
		}

		/*if (abilityState.CD > 0.0f)
		{
			abilityState.CD -= DeltaTime;

			//定时触发
			if (AbilityBarState[GetAbilityIndex(abilityState.AbilityID)] && abilityState.CD <= 0.0f)
			{
				abilityState.CD = 0.0f;
				TryActivateAbilityByID(abilityState.AbilityID);
			}
		}*/
	}

}

/*void ACharacterSV::AddAbilitys(TMap<int, int> abilitys, AActor* abilityEntity, USceneComponent* abilityEntityComp, int parentAbilityID)
{
	if (abilitys.Num() == 0) {
		return;
	}

	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	for (auto &elem : abilitys)
	{
		if (abilityInfoRes.IsExist(elem.Value))
		{
			int abilityID = elem.Value;
			AbilityList.Add(elem.Key, abilityID);

			FAbilityInfoRow &abilityInfo = abilityInfoRes.GetAbilityinfo(abilityID);
			FAbilityState abilityState;
			abilityState.AbilityID = abilityID;
			abilityState.AbilityEntity = this;
			abilityState.AbilityEntityComp = GetMesh();
			FGameplayAbilitySpec spec(abilityInfo.GameAbility->GetDefaultObject<UGameplayAbilitySV>(), 1, (int32)abilityInfo.TouchOffMode);
			if (UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(spec.Ability)) {
				abilitySV->SetAbilityInfo(abilityID, abilityInfo);
			}
			abilityState.GameplayAbilitySpec = AbilitySystemComponent->GiveAbility(spec);
			AbilityStateList.Add(abilityState);
		}
	}
}*/

void ACharacterSV::AddAbility(EAbilityBar::Type index ,int abilityID)
{
	//技能已存在无需添加
	/*if (IsExistAbility(abilityID)) {
		return;
	}*/

	if (index >= EAbilityBar::Count) {
		return;
	}

	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	if (abilityInfoRes.IsExist(abilityID))
	{
		FAbilityInfoRow &abilityInfo = abilityInfoRes.GetAbilityinfo(abilityID);
		FAbilityState abilityState;
		abilityState.AbilityID = abilityID;
		FGameplayAbilitySpec spec(abilityInfo.GameAbility->GetDefaultObject<UGameplayAbilitySV>(), 1, (int32)index);
		abilityState.GameplayAbilitySpec = AbilitySystemComponent->GiveAbility(spec);

		//初始化Ability
		UGameplayAbility* instancedAbility = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec)->GetPrimaryInstance();
		if (instancedAbility == nullptr) {
			instancedAbility = spec.Ability;
		}
		UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility);
		if (abilitySV)
		{
			abilitySV->SetAbilityInfo(abilityID, abilityInfo, GetAttribs(), TeamID, index);
			abilitySV->InitAbility();
		}
		AbilityStateList.Add(abilityState);

		AbilityList[index] = abilityID;
	}
}

bool ACharacterSV::TryActivateAbilityByIndex(int index, AActor* target, bool allowRemoteActivation)
{

	if (index < 0 || index >= EAbilityBar::Count) {
		return false;
	}
	
	int i = GetAbilityStateIndex(AbilityList[index]);
	if (i != -1)
	{
		FAbilityState &abilityState = AbilityStateList[i];
		if (abilityState.CD > 0.0f) {
			return false;
		}
		FAbilityInfoRow abilityInfo = AbilityInfoRes::Get().GetAbilityinfo(abilityState.AbilityID);
		//abilityState.CD = abilityInfo.CD;
		abilityState.IsRunning = true;

		FGameplayAbilitySpec* spec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec);
		UGameplayAbility* instancedAbility = spec->GetPrimaryInstance();
		if (instancedAbility == nullptr) {
			instancedAbility = spec->Ability;
		}
		if (UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility)) {
			abilitySV->Target = target;
		}

		//装弹不做持续按键处理
		if (index == EAbilityBar::Reload) {
			AbilityTouchOff((EAbilityBar::Type)index, false);
		}

		return AbilitySystemComponent->TryActivateAbility(abilityState.GameplayAbilitySpec, allowRemoteActivation);
	}

	return false;
}

bool ACharacterSV::TryActivateAbilityByID(int abilityID, AActor* target, bool allowRemoteActivation)
{
	if (!IsExistAbility(abilityID)) {
		return false;
	}

	int index = GetAbilityStateIndex(abilityID);
	if (index != -1)
	{
		FAbilityState &abilityState = AbilityStateList[index];
		if (abilityState.CD > 0.0f) {
			return false;
		}
		FAbilityInfoRow abilityInfo = AbilityInfoRes::Get().GetAbilityinfo(abilityState.AbilityID);
		abilityState.CD = abilityInfo.CD;
		abilityState.IsRunning = true;

		FGameplayAbilitySpec* spec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec);
		UGameplayAbility* instancedAbility = spec->GetPrimaryInstance();
		if (instancedAbility == nullptr) {
			instancedAbility = spec->Ability;
		}
		if (UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility)) {
			abilitySV->Target = target;
		}

		//装弹不做持续按键处理
		if (index == EAbilityBar::Reload) {
			AbilityTouchOff((EAbilityBar::Type)index, false);
		}

		return AbilitySystemComponent->TryActivateAbility(abilityState.GameplayAbilitySpec, allowRemoteActivation);
	}
	return false;
}

/*void ACharacterSV::ClearAbilitys(TMap<int, int> Abilitys)
{
	for (auto &elem : Abilitys)
	{
		ClearAbilityByIndex(elem.Key);
	}
}
*/
void ACharacterSV::ClearAbilityByID(int abilityID)
{
	if (!IsExistAbility(abilityID)) {
		return;
	}

	//从技能列表中删除
	int index = 0;
	for (; index < AbilityList.Num(); ++index)
	{
		if (AbilityList[index] == abilityID)
		{
			AbilityList[index] = INVALID_ABILITY;
			break;
		}
	}

	//从状态列表中删除
	for (int i = 0; i < AbilityStateList.Num(); )
	{
		FAbilityState abilityState = AbilityStateList[i];
		if (abilityState.AbilityID == abilityID)
		{
			AbilitySystemComponent->ClearAbility(abilityState.GameplayAbilitySpec);
			AbilityStateList.RemoveAt(i);
			continue;
		}

		++i;
	}

	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	FJobRow jobInfo = FJobRes::Get().GetJob(JobId);
	if (jobInfo.AbilityList.Find((EAbilityBar::Type)index)) {
		AddAbility((EAbilityBar::Type)index, jobInfo.AbilityList[(EAbilityBar::Type)index]);
	}
}

void ACharacterSV::ClearAbilityByIndex(int32 index)
{
	if (index < 0 || index >= EAbilityBar::Count) {
		return ;
	}

	int abilityID = AbilityList[index];
	AbilityList[index] = INVALID_ABILITY;

	//从状态列表中删除
	for (int i = 0; i < AbilityStateList.Num(); )
	{
		FAbilityState abilityState = AbilityStateList[i];
		if (abilityState.AbilityID == abilityID)
		{
			AbilitySystemComponent->ClearAbility(abilityState.GameplayAbilitySpec);
			AbilityStateList.RemoveAt(i);
			continue;
		}

		++i;
	}

	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	FJobRow jobInfo = FJobRes::Get().GetJob(JobId);
	if (jobInfo.AbilityList.Find((EAbilityBar::Type)index)) {
		AddAbility((EAbilityBar::Type)index, jobInfo.AbilityList[(EAbilityBar::Type)index]);
	}
}

void ACharacterSV::TickPhysics(float deltaTime)
{
	if (!bRagdoll && ImpulseTimer > .0f)
	{
		ImpulseTimer -= deltaTime;

		USkeletalMeshComponent *mesh = GetMesh();
		if (ImpulseTimer <= .0f)
		{
			mesh->SetAllBodiesPhysicsBlendWeight(0);
			mesh->SetSimulatePhysics(false);
		}
		else
		{
			float weight = gPhysicsBlendCurve.Eval(PHYSICS_BLEND_TIME - ImpulseTimer);
			mesh->SetAllBodiesPhysicsBlendWeight(weight);
		}
	}
}

void ACharacterSV::AddImpulse(const FVector &impulse, const FName &boneName, const FVector &location)
{
	USkeletalMeshComponent *mesh = GetMesh();
	if (!DisableImpulse && mesh) 
	{
		FName bone = boneName;
		if (!bRagdoll) 
		{
			FName root = mesh->GetBoneName(0);
			FName root1 = mesh->GetBoneName(1);

			if (ImpulseTimer > .0f/* || boneName == root || boneName == NAME_None*/) {
				return;
			}			

			if (boneName == root || boneName == NAME_None) {
				bone = "spine_03";
			}
		}

		ImpulseTimer = PHYSICS_BLEND_TIME;

		if (!bRagdoll)
		{
			mesh->SetAllBodiesPhysicsBlendWeight(0.0f);
			mesh->SetAllBodiesBelowSimulatePhysics(bone, true, true);
			mesh->AddImpulse(impulse, bone, true);

			ImpulseBone = bone;
		}
		else {
			mesh->AddImpulseAtLocation(impulse, location, bone);
		}		
	}
}

void ACharacterSV::Dismember(const FName &boneName, const FVector &impulse, float damage)
{
	if (DisableDismember) {
		return;
	}

	USkeletalMeshComponent *mesh = GetMesh(); 
	auto world = GetWorld();
	if (!mesh || !world) {
		return;
	}

	FLookRow &look = FLookRes::Get().GetLook(CurrLookId);
	TEnumAsByte<Dismembermemt::Type> *canDismember = look.DismemberBone.Find(boneName);
	if (canDismember)
	{
		Dismembermemt::Type type = canDismember->GetValue();
		bool *dismembered = &bDismemberedArmL;
		switch (type)
		{
		case Dismembermemt::ArmR:
			dismembered = &bDismemberedArmR;
			break;
		case Dismembermemt::Head:
			dismembered = &bDismemberedHead;
			break;
		case Dismembermemt::LegL:
			dismembered = &bDismemberedLegL;
			break;
		case Dismembermemt::LegR:
			dismembered = &bDismemberedLegR;
			break;
		}
		
		if (!*dismembered && type < look.Dismemberment.Num() && !look.Dismemberment[type].Bone.IsNone())
		{
			//initialize DismemberHealth
			if (DismemberHealth.Num() == 0)
			{
				int health = GetAttrib(EAttribType::MaxHealth);
				for (int i = 0; i < look.Dismemberment.Num(); ++i)
				{
					DismemberHealth.Add(int(health * look.Dismemberment[i].DismembermentHP));
				}
			}

			if (type < DismemberHealth.Num())
			{
				DismemberHealth[type] -= damage;
				if (DismemberHealth[type] > 0) {
					return;
				}
				DismemberHealth[type] = 0;
			}
			
			*dismembered = true;

			int breakBone = mesh->GetBoneIndex(look.Dismemberment[type].Bone);
			if (breakBone >= 0) 
			{
				mesh->HideBone(breakBone, EPhysBodyOp::PBO_Term);

				look.Dismemberment[type].Mesh.LoadSynchronous();
				if (!look.Dismemberment[type].Mesh.IsNull())
				{
					FTransform boneTrans = GetMesh()->GetBoneTransform(breakBone);

					FActorSpawnParameters params;
					params.Owner = this;
					params.Instigator = Instigator;
					ADismemberActor *part = world->SpawnActor<ADismemberActor>(ADismemberActor::StaticClass(), boneTrans, params);
					if (part)
					{
						USkeletalMesh *disMesh = look.Dismemberment[type].Mesh.Get();
						if (mesh->SkeletalMesh->Materials.Num() > look.Dismemberment[type].MaterialIndex)
						{
							disMesh->Materials.Empty();
							disMesh->Materials.Add(mesh->SkeletalMesh->Materials[look.Dismemberment[type].MaterialIndex]);
						}

						part->GetMesh()->SetSkeletalMesh(disMesh);
						part->GetMesh()->SetSimulatePhysics(true);
						part->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));						
						part->GetMesh()->AddImpulse(impulse);

						Dismembers.Add(part);
					}					
				}
			}			
		}		
	}
}

void ACharacterSV::AbilityActivatedCallBack(UGameplayAbility* ability)
{
	UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(ability);
	if (abilitySV)
	{
		ACharacterSV* caster = Cast<ACharacterSV>(ability->GetActorInfo().OwnerActor.Get());
		if (caster) {
			abilitySV->TeamID = caster->TeamID;
			caster->AbilityActivated(abilitySV->GetAbilityID());
		}
	}
}

void ACharacterSV::AbilityActivated(int abilityID)
{
	for (int i = 0; i < AbilityStateList.Num(); ++i)
	{
		if (AbilityStateList[i].AbilityID == abilityID) 
		{
			AbilityStateList[i].ActiveTime = 0.0f;
			AbilityStateList[i].IsRunning = true;
		}
	}

	OnActivateAbilitySuccess.Broadcast(abilityID);
}

void ACharacterSV::AbilityEndedCallback(UGameplayAbility* ability)
{
	UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(ability);
	if (abilitySV)
	{
		ACharacterSV* character = Cast<ACharacterSV>(ability->GetActorInfo().OwnerActor.Get());
		if (character) {
			character->AbilityEnded(abilitySV->GetAbilityID());
		}
	}
}

void ACharacterSV::AbilityEnded(int abilityID)
{

	for (int i = 0; i < AbilityStateList.Num(); ++i)
	{
		if (AbilityStateList[i].AbilityID == abilityID)
		{
			FAbilityInfoRow info = AbilityInfoRes::Get().GetAbilityinfo(abilityID);
			AbilityStateList[i].IsRunning = false;
			if (info.AbilityAnimation <= 0.0f || AbilityStateList[i].ActiveTime >= info.AbilityAnimation) {
				AbilityStateList[i].CD = info.CD;
			}

			/*int parentAbilityID = AbilityStateList[i].ParentAbilityID;
			if (parentAbilityID != 0)
			{

				bool isClearAbility = false;
				if (!IsExistAbility(parentAbilityID)) {
					isClearAbility = true;
				}
				else
				{
					FAbilityState parentAbilityState = AbilityStateList[GetAbilityStateIndex(parentAbilityID)];
					isClearAbility = !parentAbilityState.IsRunning;
				}
				if (isClearAbility) {
					ClearAbilityByID(abilityID);
				}

			}*/
			break;
		}
	}
	OnActivateAbilityFinished.Broadcast(abilityID);
}

void ACharacterSV::AbilityTouchOff(EAbilityBar::Type index, bool touchOff, AActor* target, bool cancleAbility)
{
	if (index >= EAbilityBar::Count) {
		return;
	}

	if (AbilityList[index] == INVALID_ABILITY) {
		return;
	}

	if (Role == ROLE_Authority)
	{
		AbilityBarState[index] = touchOff;
		//TouchOffState[index] = touchOff;

		if (AbilityList[index] == INVALID_ABILITY) {
			return;
		}

		if (!touchOff)
		{
			if (cancleAbility) {
				EndOrCancelAbilityByIndex(index, true);
			}
			return;
		}

		AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
		int abilityID = AbilityList[index];
		if (abilityInfoRes.IsExist(abilityID)) {
			TryActivateAbilityByIndex(index, target);
		}
	}
	else if (Role == ROLE_AutonomousProxy) {
		ServeAbilityTouchOff(index, touchOff, target);
		UE_LOG(LogTemp, Warning, TEXT("ServeAbilityTouchOff..."));
	}
	
}
bool ACharacterSV::ServeAbilityTouchOff_Validate(EAbilityBar::Type index, bool touchOff, AActor* target, bool cancleAbility)
{
	return true;
}


void ACharacterSV::ServeAbilityTouchOff_Implementation(EAbilityBar::Type index, bool touchOff, AActor* target, bool cancleAbility)
{
	AbilityBarState[index] = touchOff;
	//TouchOffState[index] = touchOff;

	if (AbilityList[index] == INVALID_ABILITY) {
		return;
	}

	if (!touchOff) 
	{
		if (cancleAbility) {
			EndOrCancelAbilityByIndex(index, true);
		}
		return;
	}

	AbilityInfoRes abilityInfoRes = AbilityInfoRes::Get();
	int abilityID = AbilityList[index];
	if (abilityInfoRes.IsExist(abilityID)) {
		TryActivateAbilityByIndex(index, target);
	}
}

void ACharacterSV::EndOrCancelAbilityByID(int abilityID, bool bWantCancel)
{
	if (!IsExistAbility(abilityID)) {
		return;
	}

	FAbilityState abilityState = GetAbilityStateByID(abilityID);
	if (abilityState.IsRunning == false) {
		return;
	}

	FGameplayAbilitySpec* spec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec);
	UGameplayAbility* instancedAbility = spec->GetPrimaryInstance();
	if (instancedAbility == nullptr) {
		instancedAbility = spec->Ability;
	}

	UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility);

	if (abilitySV)
	{
		if (bWantCancel)
		{
			abilitySV->CancelAbility(abilityState.GameplayAbilitySpec, AbilitySystemComponent->AbilityActorInfo.Get(), spec->ActivationInfo, true);
		}
		else
		{
			abilitySV->EndAbility(abilityState.GameplayAbilitySpec, AbilitySystemComponent->AbilityActorInfo.Get(), spec->ActivationInfo, true, bWantCancel);
		}
	}
}

void ACharacterSV::EndOrCancelAbilityByIndex(int index, bool bWantCancel)
{
	if (index < 0 || index >= EAbilityBar::Count) {
		return ;
	}

	FAbilityState abilityState = GetAbilityStateByIndex(index);
	if (abilityState.IsRunning == false) {
		return;
	}

	FGameplayAbilitySpec* spec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityState.GameplayAbilitySpec);
	UGameplayAbility* instancedAbility = spec->GetPrimaryInstance();
	if (instancedAbility == nullptr) {
		instancedAbility = spec->Ability;
	}

	UGameplayAbilitySV* abilitySV = Cast<UGameplayAbilitySV>(instancedAbility);

	if (abilitySV)
	{
		if (bWantCancel)
		{
			abilitySV->CancelAbility(abilityState.GameplayAbilitySpec, AbilitySystemComponent->AbilityActorInfo.Get(), spec->ActivationInfo, true);
		}
		else
		{
			abilitySV->EndAbility(abilityState.GameplayAbilitySpec, AbilitySystemComponent->AbilityActorInfo.Get(), spec->ActivationInfo, true, bWantCancel);
		}
	}
}

//技能接口
//////////////////////////////////////////////////////////////////////////
void ACharacterSV::GetFromActorAndMesh(AActor *fromActor, EEffectPosition::Type fromPosition, AActor *&actor, UMeshComponent *&meshComp)
{
	actor = nullptr;
	meshComp = nullptr;
	ACharacterSV *chSV = nullptr;
	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::MeshAttached:
		actor = fromActor;
		chSV = Cast<ACharacterSV>(fromActor);
		if (chSV) {
			meshComp = chSV->GetMesh();
		}
		else {
			meshComp = Cast<UMeshComponent>(actor->GetComponentByClass(UMeshComponent::StaticClass()));
		}
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::ActorAttached:
		actor = fromActor;
		break;
	case EEffectPosition::GripMesh:
	case EEffectPosition::GripMeshAttached:
		chSV = Cast<ACharacterSV>(fromActor);
		if (chSV)
		{
			meshComp = chSV->GetSpecMesh(ESpecMesh::GripActor);
			if (meshComp) {
				actor = meshComp->GetOwner();
			}
		}
		break;
	case EEffectPosition::GripActor:
	case EEffectPosition::GripActorAttached:
		chSV = Cast<ACharacterSV>(fromActor);
		if (chSV) {
			actor = chSV->GetSpecActor(ESpecMesh::GripActor);
		}
		break;
	default:
		break;
	}
}

FTransform ACharacterSV::MakeFromTransform(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relaLocation, FRotator relaRotation)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	FTransform trans;
	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMesh:
	case EEffectPosition::GripMeshAttached:
		if (mesh) {
			trans = mesh->GetSocketTransform(fromSocket);
		}
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActor:
	case EEffectPosition::GripActorAttached:
		if (actor) {
			trans = actor->GetTransform();
		}
	default:
		break;
	}

	FTransform rela(relaRotation, relaLocation);

	FTransform ret;
	FTransform::Multiply(&ret, &rela, &trans);
	return ret;
}

UParticleSystemComponent* ACharacterSV::PlayParticleInternal(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation, bool bAutoDestroy)
{
	UParticleSystemComponent* psc = nullptr;
	FParticleRow &particle = ParticleRes::Get().GetParticle(particleID);
	if (particle.Particle == nullptr) {
		return psc;
	}

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::GripMesh:
		if (mesh) {
			FRotator RotatorOffset = UKismetMathLibrary::ComposeRotators(particle.Rotator, relativeRotation);
			FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, RotatorOffset);
			FVector loc = trans.GetLocation();
			FRotator rot = trans.Rotator();
			psc = UGameplayStatics::SpawnEmitterAtLocation(fromActor, particle.Particle, loc, rot, bAutoDestroy);
		}
		break;
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMeshAttached:
		if (mesh) {
			psc = UGameplayStatics::SpawnEmitterAttached(particle.Particle, mesh, fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, bAutoDestroy);
		}
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::GripActor:
		if (actor) {
			FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
			psc = UGameplayStatics::SpawnEmitterAtLocation(fromActor, particle.Particle, trans.GetLocation(), trans.Rotator(), bAutoDestroy);
		}
		break;
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActorAttached:
		if (actor) {
			psc = UGameplayStatics::SpawnEmitterAttached(particle.Particle, actor->GetRootComponent(), fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, bAutoDestroy);
		}
		break;
	default:
		break;
	}

	if (psc) {
		psc->CustomTimeDilation = particle.TimeDilation;
		psc->SetWorldScale3D(particle.Scale);
	}

	return psc;
}

UAudioComponent* ACharacterSV::PlaySoundInternal(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation)
{
	UAudioComponent* ac = nullptr;
	FSoundRow &sound = SoundRes::Get().GetSound(soundID);
	if (sound.Sound == nullptr) {
		return ac;
	}

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::GripMesh:
		if (mesh) {
			FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
			FVector loc = trans.GetLocation();
			FRotator rot = trans.Rotator();
			ac = UGameplayStatics::SpawnSoundAtLocation(fromActor, sound.Sound, loc, rot, sound.Volume, sound.Pitch);
		}
		break;
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMeshAttached:
		if (mesh) {
			ac = UGameplayStatics::SpawnSoundAttached(sound.Sound, mesh, fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, true, sound.Volume, sound.Pitch);
		}
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::GripActor:
		if (actor) {
			FTransform trans = MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
			ac = UGameplayStatics::SpawnSoundAtLocation(fromActor, sound.Sound, trans.GetLocation(), trans.Rotator(), sound.Volume, sound.Pitch);
		}
		break;
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActorAttached:
		if (actor) {
			ac = UGameplayStatics::SpawnSoundAttached(sound.Sound, actor->GetRootComponent(), fromSocket, relativeLocation, relativeRotation, EAttachLocation::KeepRelativeOffset, true, sound.Volume, sound.Pitch);
		}
		break;
	default:
		break;
	}

	if (ac)
	{
		ac->bOverrideAttenuation = true;
		ac->AttenuationOverrides.AttenuationShapeExtents.X = sound.Radius;
		ac->AttenuationOverrides.FalloffDistance = sound.FalloffDistance;
	}

	return ac;
}

FVector ACharacterSV::GetTargetLocationAndBone(AActor* target, FName &boneName)
{
	boneName.SetNumber(0);
	if (target)
	{
		USkeletalMeshComponent* skeletalMesh = Cast<USkeletalMeshComponent>(target->FindComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (skeletalMesh)
		{
			UPhysicsAsset* physicsAsset = skeletalMesh->GetPhysicsAsset();
			if (physicsAsset)
			{
				int num = physicsAsset->SkeletalBodySetups.Num();
				if (num > 0)
				{
					int index = FMath::Rand() % num;
					boneName = physicsAsset->SkeletalBodySetups[index]->BoneName;
					return skeletalMesh->GetSocketLocation(boneName);
				}
			}
		}

		return target->GetActorLocation();
	}
	
	return FVector::ZeroVector;
}

void ACharacterSV::K2_AimTarget(AActor* target, FName &boneName)
{
	AimTarget(target, boneName);
}

void ACharacterSV::AimTarget(AActor* target, FName &boneName)
{
}

bool ACharacterSV::NeedReload()
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(this, EEffectPosition::GripActor, actor, mesh);

	AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
	if (entity == nullptr) {
		return false;
	}

	return entity->NeedReload();
}

void ACharacterSV::K2_NotifyOnHit_Implementation(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType)
{
}

void ACharacterSV::NotifyOnHit(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType)
{
	

	FVector dir = hit.TraceEnd - hit.TraceStart;
	dir = dir.GetSafeNormal();

	switch (attackType)
	{
	case EAttackType::Projectile:
		Dismember(hit.BoneName, dir, damageValue);
		AddImpulse(dir*0.1f, hit.BoneName, hit.Location);
		break;
	case EAttackType::Bomb:
		dir = GetActorLocation() - hit.TraceStart;
		dir = dir.GetSafeNormal();
		if (CurHealth <= 0){
			AddImpulse(dir*30000.0f, hit.BoneName, hit.Location);
		}

		//GetMesh()->AddImpulse(dir*500.0f);
		break;
	}

	if (CurHealth <= 0) {
		NotifyDead(hit, attackType);
	}

	K2_NotifyOnHit(hit, damageValue, CurHealth, attackType);
}

void ACharacterSV::NetMulticastNotifyOnHit_Implementation(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType)
{
	NotifyOnHit(hit, damageValue, CurHealth, attackType);
}

void ACharacterSV::NetMulticastNotifyDead_Implementation(const FHitResult &hit, EAttackType::Type attackType)
{
	NotifyDead(hit, attackType);
}

void ACharacterSV::K2_NotifyDead_Implementation(const FHitResult &hit, EAttackType::Type attackType)
{

}

void ACharacterSV::NotifyDead(const FHitResult &hit, EAttackType::Type attackType)
{
	K2_NotifyDead(hit, attackType);

	//死亡后取消所有技能
	for (int i = 0; i < AbilityStateList.Num(); ++i)
	{
		if (AbilityStateList[i].IsRunning) {
			EndOrCancelAbilityByID(AbilityStateList[i].AbilityID, true);
		}
	}

	SetDead(true);
	if (!bRagdoll)
	{
		bRagdoll = true;

		USkeletalMeshComponent *mesh = GetMesh();

		mesh->SetSimulatePhysics(true);
		mesh->SetEnableGravity(true);
		PhysicalAnimation->ApplyPhysicalAnimationProfileBelow(NAME_None, NAME_None, true);
		mesh->SetConstraintProfileForAll(NAME_None, true);

		mesh->SetPhysicsBlendWeight(1.0f);
		mesh->SetCollisionProfileName("Ragdoll");
	}	
}

void ACharacterSV::NetMulticastPlayParticle_Implementation(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation)
{
	PlayParticleInternal(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
}

void ACharacterSV::NetMulticastPlayParticleTrail_Implementation(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation
	, FName toParamName, const FVector &toLocation)
{
	UParticleSystemComponent* psc = PlayParticleInternal(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
	if (psc) {
		psc->SetVectorParameter(toParamName, toLocation);
	}
}

void ACharacterSV::NetMulticastPlaySound_Implementation(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation)
{
	PlaySoundInternal(soundID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
}

void ACharacterSV::NetMulticastSpawnImpactEffects_Implementation(const FHitResult& hit, TSubclassOf<AHitImpactEffect> impactEffect)
{
	if (impactEffect && hit.bBlockingHit)
	{
		FTransform const SpawnTransform(hit.ImpactNormal.Rotation(), hit.ImpactPoint);
		AHitImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AHitImpactEffect>(impactEffect, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = hit;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
}

void ACharacterSV::NetMulticastCalcShakeOffset_Implementation(AActor *fromActor, EEffectPosition::Type fromPosition)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	if (mesh == nullptr) {
		return;
	}
	AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
	if (entity == nullptr) {
		return;
	}

	FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(entity->GetParamID());
	FShakeRow shake = ShakeRes::Get().GetShake(param.ShakeID);
	float curRecoilPitch = UKismetMathLibrary::RandomFloatInRange(shake.PitchMin, shake.PitchMax);
	float curRecoilYaw = UKismetMathLibrary::RandomFloatInRange(shake.YawMin, shake.YawMax);
	if (UKismetMathLibrary::RandomBool()) {
		curRecoilYaw *= -1;
	}

	FTransform currentShakeOffset = entity->GetShakeOffset();
	FTransform goldShakeOffset;


	FVector locationOffset = currentShakeOffset.GetLocation() + FVector(shake.XPush, 0, 0);
	locationOffset = UKismetMathLibrary::ClampVectorSize(locationOffset, 0, FMath::Abs(shake.MaxTotalXPushSize));
	goldShakeOffset.SetLocation(locationOffset);

	FRotator rotatorOffset = UKismetMathLibrary::ComposeRotators(currentShakeOffset.GetRotation().Rotator(), FRotator(curRecoilPitch, curRecoilYaw, 0));
	float limitPitch = shake.MaxTotalPitch + UKismetMathLibrary::RandomFloatInRange(-shake.MaxTotalPitchVariance, shake.MaxTotalPitchVariance);
	rotatorOffset.Pitch = FMath::Min<float>(rotatorOffset.Pitch, FMath::Max<float>(0, limitPitch));
	rotatorOffset.Yaw = UKismetMathLibrary::FClamp(rotatorOffset.Yaw, -shake.MaxTotalYaw, shake.MaxTotalYaw);
	goldShakeOffset.SetRotation(FQuat(rotatorOffset));

	entity->SetGoldShakeOffset(goldShakeOffset);
}

/*void ACharacterSV::NetMulticastPlayMontage_Implementation(USkeletalMeshComponent* skeletalMesh, UAnimMontage* montage, float rate, FName startSection)
{
	if (!skeletalMesh || !montage) {
		return;
	}

	UAnimInstanceBase* animInstance = Cast<UAnimInstanceBase>(skeletalMesh->GetAnimInstance());
	if (!animInstance) {
		return;
	}

	animInstance->Montage_Play(montage, rate);
	
	if (startSection != NAME_None) {
		animInstance->Montage_JumpToSection(startSection, montage);
	}
}*/

void ACharacterSV::NetMulticastPlayMontage_Implementation(USkeletalMeshComponent* skeletalMesh, EAnimAction::Type actionType, EEffectPosition::Type fromPosition, UAbilityTask_PlayMontageAndWaitSV* ability)
{
	if (!skeletalMesh) {
		return;
	}

	UAnimInstanceBase* animInstance = Cast<UAnimInstanceBase>(skeletalMesh->GetAnimInstance());
	if (!animInstance) {
		return;
	}

	bool gripMesh = false;
	if (fromPosition == EEffectPosition::GripMesh || fromPosition == EEffectPosition::GripMeshAttached) {
		gripMesh = true;
	}
	
	if (gripMesh && GripMontageStop) 
	{
		GripMontageToplay = nullptr;
		GripMontageStop = false;
		return;
	}

	if (!gripMesh && MontageStop) 
	{
		MontageToPlay = nullptr;
		MontageStop = false;
		return;
	}

	UAnimMontage* montage = animInstance->PlayAction(actionType);
	if (gripMesh) {
		GripMontageToplay = montage;
	}
	else {
		MontageToPlay = montage;
	}

	if (Role == ROLE_Authority && ability) {
		ability->SetActionStart(montage);
	}
}
/*
void ACharacterSV::NetMulticastStopMontage_Implementation(USkeletalMeshComponent* skeletalMesh, UAnimMontage* montage)
{
	if (!skeletalMesh || !montage) {
		return;
	}

	UAnimInstanceBase* animInstance = Cast<UAnimInstanceBase>(skeletalMesh->GetAnimInstance());
	if (!animInstance) {
		return;
	}

	animInstance->Montage_Stop(montage->BlendOut.GetBlendTime(), montage);
}
*/

void ACharacterSV::NetMulticastStopMontage_Implementation(USkeletalMeshComponent* skeletalMesh, EEffectPosition::Type fromPosition)
{
	if (!skeletalMesh) {
		return;
	}

	UAnimInstanceBase* animInstance = Cast<UAnimInstanceBase>(skeletalMesh->GetAnimInstance());
	if (!animInstance) {
		return;
	}

	bool gripMesh = false;
	if (fromPosition == EEffectPosition::GripMesh || fromPosition == EEffectPosition::GripMeshAttached) {
		gripMesh = true;
	}

	UAnimMontage* montage = MontageToPlay;
	if (gripMesh) {
		montage = GripMontageToplay;
	}

	if (montage)
	{
		animInstance->Montage_Stop(montage->BlendOut.GetBlendTime(), montage);
	}
	else
	{
		if (gripMesh) {
			GripMontageStop = true;
		}
		else {
			MontageStop = true;
		}
	}
}

ACharacterSV *ACharacterSV::GetAITarget() const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(Controller);
	if (controller) {
		return controller->GetTarget();
	}
	return NULL;
}

void ACharacterSV::ActiveSpline(bool active, float newVelocity, bool showMesh)
{
	bTraceSpline = active;
	if (bTraceSpline) {
		Velocity = newVelocity;
		bShowSplineMesh = showMesh;
		UpdateSplinePath();
	}
	else {
		ClearSplinePath();
	}
}

void ACharacterSV::ClearSplinePath()
{
	SplineComp->ClearSplinePoints();

	for (int i = 0; i < SplineMeshs.Num(); ++i)
	{
		SplineMeshs[i]->SetHiddenInGame(true);
	}
}

void ACharacterSV::UpdateSplinePath()
{
	ClearSplinePath();
	
	AActor* actor;
	UMeshComponent* mesh;
	GetFromActorAndMesh(this, EEffectPosition::GripActor, actor, mesh);
	AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
	if (entity == nullptr) {
		return;
	}

	FVector startTrace = entity->GetTraceWorldLocation();
	LaunchVelocity = FRotationMatrix(entity->GetTraceWorldRotation()).GetScaledAxis(EAxis::X) * Velocity;

	FPredictProjectilePathParams Params = FPredictProjectilePathParams(0.0f, startTrace, LaunchVelocity, 2.0f);
	Params.bTraceWithCollision = true;
	//Params.DrawDebugType = DrawDebugType;

	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TRACE_FIRE);
	Params.TraceChannel = CollisionChannel;
	Params.ActorsToIgnore.Add(this);
	Params.ActorsToIgnore.Add(entity);

	FPredictProjectilePathResult PredictResult;
	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, PredictResult);

	FTransform ActorTrans = GetActorTransform();
	for (int i = 0; i < PredictResult.PathData.Num(); ++i)
	{
		FVector relativeLoction = ActorTrans.InverseTransformPosition(PredictResult.PathData[i].Location);
		SplineComp->AddSplinePoint(relativeLoction, ESplineCoordinateSpace::Local);
	}
	SplineComp->SetSplinePointType(PredictResult.PathData.Num() - 1, ESplinePointType::CurveClamped);

	for (int i = 0; i < SplineComp->GetNumberOfSplinePoints() - 2; ++i)
	{
		if (i >= SplineMeshs.Num())
		{
			USplineMeshComponent* comp = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			comp->RegisterComponent();

			comp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			comp->SetMobility(EComponentMobility::Movable);
			comp->SetStaticMesh(SplineStaticMesh);
			comp->AttachToComponent(SplineComp, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));

			SplineMeshs.Add(comp);
		}
		else
		{
			SplineMeshs[i]->SetHiddenInGame(false);
		}

		FVector startPos, startTangent, endPos, endTangent;
		SplineComp->GetLocationAndTangentAtSplinePoint(i, startPos, startTangent, ESplineCoordinateSpace::Local);
		SplineComp->GetLocationAndTangentAtSplinePoint(i + 1, endPos, endTangent, ESplineCoordinateSpace::Local);

		SplineMeshs[i]->SetStartAndEnd(startPos, startTangent, endPos, endTangent);
	}

	//RegisterAllComponents();
}

FVector ACharacterSV::GetSplineLaunchVelocity()
{
	return LaunchVelocity;
}


