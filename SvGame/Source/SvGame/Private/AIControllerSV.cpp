// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"

#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

#include "Kismet/KismetMathLibrary.h"

#include "AIPerceptionComponentSV.h"
#include "AISenseConfig_Attract.h"
#include "AISenseConfig_PenetrateSight.h"
#include "BTTask_BlueprintBaseSV.h"
#include "AIControllerSV.h"
#include "BTRes.h"
#include "JobRes.h"
#include "GameModeSVBattle.h"

AAIControllerSV::AAIControllerSV(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bSeamlessTravel = false;
	AISpeedType = ESpeedType::None;
}

void AAIControllerSV::SetTarget(ACharacterSV *Target, bool bChangeState, bool bSendQueuedObserverNotifications)
{
	if (Target)
	{
		if (Blackboard)
		{
			AActor *oldTarget = Cast<AActor>(Blackboard->GetValueAsObject("Target"));
			if (oldTarget != Target) 
			{
				if (oldTarget || TargetDeadDelegateHandle.IsValid()) {
					SetTarget(NULL);
				}

				TargetDeadDelegateHandle = Target->OnCharacterDead.AddUObject(this, &AAIControllerSV::OnTargetDeadCallBack);

				Blackboard->PauseObserverNotifications();
				Blackboard->SetValueAsObject("Target", Target);
				Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
			}
		}
	}
	else
	{
		if (Blackboard)
		{
			ACharacterSV *oldTarget = Cast<ACharacterSV>(Blackboard->GetValueAsObject("Target"));
			if (oldTarget && TargetDeadDelegateHandle.IsValid())
			{
				oldTarget->OnCharacterDead.Remove(TargetDeadDelegateHandle);
				TargetDeadDelegateHandle.Reset();
			}
			
			Blackboard->PauseObserverNotifications();
			Blackboard->SetValueAsObject("Target", NULL);
			if (bChangeState) {
				ChangeAIState(EStateType::Idle);
			}
			Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
		}
	}
}

ACharacterSV *AAIControllerSV::GetTarget()
{
	if (Blackboard) {
		return Cast<ACharacterSV>(Blackboard->GetValueAsObject("Target"));
	}
	return NULL;
}

void AAIControllerSV::OnPerceptionUpdated(TArray<AActor*> UpdatedActors)
{
	if (Blackboard)
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(GetPawn());
		if (NULL == pawn)
			return;

		if (Blackboard->GetValueAsEnum("State") == EStateType::Disable) 
		{
			for (int i = 0; i < UpdatedActors.Num(); ++i)
			{
				ACharacterSV *actor = Cast<ACharacterSV>(UpdatedActors[i]);
				if (actor && pawn->TeamID != actor->TeamID)
				{
					ChangeAIState(EStateType::Idle);
					return;
				}
			}
		}
	}
}

void AAIControllerSV::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	//ACharacterSV *pawn = Cast<ACharacterSV>(Actor);
	//if (NULL == pawn)
	//	return;

	if (Blackboard)
	{
		if (Blackboard->GetValueAsEnum("State") == EStateType::Disable || Blackboard->GetValueAsEnum("State") == EStateType::Patrol)
		{
			FAIExtra &extra = GetAIExtra();
			if (!extra.ActiveType.IsSight() && (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() || Stimulus.Type == UAISense::GetSenseID<UAISense_PenetrateSight>())) {
				return;
			}

			if (Stimulus.WasSuccessfullySensed() && FGenericTeamId::GetAttitude(GetPawn(), Actor) == ETeamAttitude::Hostile && CheckStimulusType(Stimulus.Type))
			{
				Blackboard->PauseObserverNotifications();
				ChangeAIState(EStateType::Idle);
				if (extra.ActiveType.IsSight() && extra.SightAngle != 180 && GetPerceptionComponent())
				{
					UAISenseConfig_Sight *sight = Cast<UAISenseConfig_Sight>(GetPerceptionComponent()->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()));
					if (sight) 
					{
						sight->PeripheralVisionAngleDegrees = 180;
						GetPerceptionComponent()->ConfigureSense(*sight);
					}
				}

				Blackboard->ResumeObserverNotifications(true);
				return;
			}
		}
		
		if (Blackboard->GetValueAsEnum("State") != EStateType::Disable && Blackboard->GetValueAsEnum("State") != EStateType::Patrol)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				if (FGenericTeamId::GetAttitude(GetPawn(), Actor) == ETeamAttitude::Hostile)
				{
					FAIExtra &extra = GetAIExtra();
					if (extra.ActiveType.IsAttract())
					{
						ACharacterSV *target = Cast<ACharacterSV>(Actor);
						ACharacterSV *oldTarget = Cast<ACharacterSV>(Blackboard->GetValueAsObject("Target"));
						if (target && target->IsMinionAttract() && target != oldTarget)
						{
							if (oldTarget && !oldTarget->IsMinionAttract()) {
								SetTarget(NULL, true, true);
							}

							if (NULL == oldTarget || !oldTarget->IsMinionAttract()) {
								SetTarget(target, false, false);
							}
						}
					}
				}
			}
			else
			{
				ACharacterSV *target = Cast<ACharacterSV>(Blackboard->GetValueAsObject("Target"));
				if (target && target == Actor)
				{
					if (GetPerceptionComponent())
					{
						const FActorPerceptionInfo* perceptionInfo = GetPerceptionComponent()->GetActorInfo(*Actor);
						if (!target->IsMinionAttract() && !perceptionInfo->HasAnyCurrentStimulus()) {
							SetTarget(NULL, true, true);
						}
					}
				}
			}
		}
	}
}

void AAIControllerSV::OnTargetDeadCallBack(ACharacterSV *Target)
{
	SetTarget(NULL, true, true);
}

bool AAIControllerSV::CheckStimulusType(FAISenseID Type)
{
	bool ret = false;

	FAIExtra &extra = GetAIExtra();
	if (extra.ActiveType.IsSight()) {
		ret = Type == UAISense::GetSenseID<UAISense_Sight>() || Type == UAISense::GetSenseID<UAISense_PenetrateSight>();
	}

	if (!ret && extra.ActiveType.IsHearing()) {
		ret = Type == UAISense::GetSenseID<UAISense_Hearing>();
	}

	if (!ret && extra.ActiveType.IsDamage()) {
		ret = Type == UAISense::GetSenseID<UAISense_Damage>();
	}

	if (!ret && extra.ActiveType.IsAttract()) {
		ret = Type == UAISense::GetSenseID<UAISense_Attract>();
	}

	return ret;
}

void AAIControllerSV::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	/*if (bStopAILogicOnUnposses)
	{
		UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
		if (BTComp) {
			BTComp->RestartTree();
		}
	}*/
	
	Init();
}

void AAIControllerSV::UnPossess()
{
	Super::UnPossess();

	if (GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAIControllerSV::OnTargetPerceptionUpdated);
		GetPerceptionComponent()->DestroyComponent();
	}
}

void AAIControllerSV::Init()
{
	ACharacterSV *pawn = Cast<ACharacterSV>(GetPawn());
	if (pawn)
	{
		FAIExtra &extra = GetAIExtra();
		DefaultNavigationFilterClass = extra.NavQueryFilter;

		//init perception
		//if (!pawn->IsMajor())
		if (GetPerceptionComponent())
		{
			GetPerceptionComponent()->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAIControllerSV::OnTargetPerceptionUpdated);
			GetPerceptionComponent()->DestroyComponent();
		}

		SetPerceptionComponent(*NewObject<UAIPerceptionComponentSV>(this, TEXT("PerceptionComponent")));

		if (extra.SightType != ESightType::None)
		{
			UAISenseConfig_Sight *sight = extra.SightType == ESightType::Penetrate ? NewObject<UAISenseConfig_PenetrateSight>(this, FName("Penetrate Sight Config"))
				                                                : NewObject<UAISenseConfig_Sight>(this, FName("Sight Config"));

			sight->SightRadius = extra.WarnRadius;
			sight->LoseSightRadius = extra.WarnRadius * 1.5f;
			sight->PeripheralVisionAngleDegrees = extra.ActiveType.IsSight() ? extra.SightAngle : 180;
			sight->AutoSuccessRangeFromLastSeenLocation = 1000;

			sight->DetectionByAffiliation.bDetectEnemies = true;
			sight->DetectionByAffiliation.bDetectFriendlies = false;//pawn->IsMajor();
			sight->DetectionByAffiliation.bDetectNeutrals = false;//pawn->IsMajor();

			GetPerceptionComponent()->ConfigureSense(*sight);
		}

		if (extra.ActiveType.IsHearing())
		{
			UAISenseConfig_Hearing *hearing = NewObject<UAISenseConfig_Hearing>(this, FName("Hearing Config"));
			hearing->DetectionByAffiliation.bDetectEnemies = true;
			hearing->DetectionByAffiliation.bDetectFriendlies = false;
			hearing->DetectionByAffiliation.bDetectNeutrals = false;

			GetPerceptionComponent()->ConfigureSense(*hearing);
		}

		if (extra.ActiveType.IsAttract())
		{
			UAISenseConfig_Attract *attract = NewObject<UAISenseConfig_Attract>(this, FName("Attract Config"));

			attract->DetectionByAffiliation.bDetectEnemies = true;
			attract->DetectionByAffiliation.bDetectFriendlies = false;
			attract->DetectionByAffiliation.bDetectNeutrals = false;

			GetPerceptionComponent()->ConfigureSense(*attract);
		}

		if (extra.ActiveType.IsDamage())
		{
			UAISenseConfig_Damage *damage = NewObject<UAISenseConfig_Damage>(this, FName("Damage Config"));
			GetPerceptionComponent()->ConfigureSense(*damage);
		}

		GetPerceptionComponent()->RegisterComponent();
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerSV::OnTargetPerceptionUpdated);

		//init behaviortree
		const FJobRow &Job = FJobRes::Get().GetJob(pawn->JobId);
		RunBehaviorTreeByID(Job.BTId);

		if (Blackboard) {
			Blackboard->PauseObserverNotifications();
		}

		InitializeBlackboardData();

		if (Blackboard) {
			Blackboard->ResumeObserverNotifications(false);
		}

		InitTargetPoint();
	}
}

void AAIControllerSV::InitTargetPoint()
{
	TargetPoints.Reset();
	CurrTargetPoint = 1;
	CurrMasterPathIndex = 0;

	FAIExtra &AIExtra = GetAIExtra();
	if (AIExtra.TargetPointNames.Num() > 0)
	{
		AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			for (int i = 0; i < AIExtra.TargetPointNames.Num(); ++i)
			{
				ATargetPointSV **targetPointPtr = GameMode->TargetPoints.Find(AIExtra.TargetPointNames[i]);
				if (targetPointPtr) {
					TargetPoints.Add(*targetPointPtr);
				}
			}
		}
	}
}

bool AAIControllerSV::RunBehaviorTreeByID(int BTId)
{
	UBehaviorTree *BTAsset = nullptr;
	FBTRes::Get().CreateBT(BTId, BTAsset);
	if (BTAsset) {
		return RunBehaviorTree(BTAsset);
	}

	return false;
}

void AAIControllerSV::UpdateBlackboardAttribValue(EAttribType::Type KeyID)
{
	if (Blackboard && PlayerState)
	{
		/*
		Blackboard->SetValueAsFloat(ATTRIBUTE_NAME[KeyID], PlayerState->Attrib[KeyID]);
		*/
	}
}

void AAIControllerSV::ChangeAIState(EStateType::Type State)
{
	if (Blackboard && Blackboard->GetValueAsEnum("State") != State)
	{
		OnStateOut();
		Blackboard->SetValueAsEnum("State", State);
		OnStateEnter();
	}
}

void AAIControllerSV::OnStateOut()
{
	if (Blackboard)
	{
		if (Blackboard->GetValueAsEnum("State") == EStateType::Attack
			|| Blackboard->GetValueAsEnum("State") == EStateType::Rescue
			|| Blackboard->GetValueAsEnum("State") == EStateType::Cure)
		{
			Blackboard->SetValueAsBool("bOutSkillRadius", false);
			Blackboard->SetValueAsInt("AbilityID", 0);
		}
	}
}

void AAIControllerSV::OnStateEnter()
{

}

void AAIControllerSV::InitializeBlackboardData()
{
	if (Blackboard)
	{
		/*
		if (PlayerState)
		{
			for (int i = 0; i < EAttribType::Count; ++i)
			{
				Blackboard->SetValueAsFloat(ATTRIBUTE_NAME[i], PlayerState->Attrib[i]);
			}
		}
		*/
		FAIExtra &AIExtra = GetAIExtra();
		if (Blackboard->GetValueAsEnum("State") == EStateType::None) {
			ChangeAIState(AIExtra.OriginState);
		}
		else {
			ChangeAIState(EStateType::Idle);
		}
	}
}

ACharacterSV* AAIControllerSV::GetMajorNearest(bool bBot, int Radius)
{
	ACharacterSV *MajorChar = nullptr;

	AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(GetWorld()->GetAuthGameMode());
	if (GameMode) 
	{
		FVector Location = GetPawn()->GetActorLocation();

		float NearestDis = FLT_MAX;
		for (auto It = GameMode->CreatePawnConstIterator(); It; ++It)
		{
			if ((*It)->Controller && (bBot || (!bBot && (*It)->Controller->IsPlayerController())))
			{
				APawn *OtherPawn = (*It)->Controller->GetPawn();
				if (OtherPawn)
				{
					FVector Dis = OtherPawn->GetActorLocation() - Location;
					if (Dis.Size() < NearestDis && (0 == Radius || Dis.Size() < Radius))
					{
						NearestDis = Dis.Size();
						MajorChar = Cast<ACharacterSV>(OtherPawn);
					}
				}
			}
		}
	}

	return MajorChar;
}

FAIExtra& AAIControllerSV::GetAIExtra()
{
	ACharacterSV *PossessPawn = Cast<ACharacterSV>(GetPawn());
	if (PossessPawn) {
		return PossessPawn->AIExtra;
	}
	return DefaultAIExtra;
}

static float GetRadiusFromDirection(float distance)
{
	if (distance > 2000) {
		return 1000;
	}
	else if (distance > 500) {
		return 400;
	}
	else if (distance > 200) {
		return 100;
	}
	return 0;
}

bool AAIControllerSV::GetRandomReachablePointInRadius(const FVector &OrginLocation, FVector &GoalLocation, float Radius) const
{
	ACharacterSV *pawn = Cast<ACharacterSV>(GetPawn());
	if (pawn)
	{
		float radius = Radius;
		FVector orginLocation = OrginLocation;

		if (Radius < 0)
		{
			FVector direction = pawn->GetActorLocation() - OrginLocation;
			radius = GetRadiusFromDirection(direction.Size());// direction.Size() > 500 ? 400 : 0;
			orginLocation = OrginLocation + (direction.GetSafeNormal() * radius);
		}

		if (radius)
		{
			UNavigationSystem* navSys = UNavigationSystem::GetCurrent(GetWorld());
			if (navSys)
			{
				ANavigationData* navData = navSys->GetNavDataForProps(GetNavAgentPropertiesRef());
				if (navData == nullptr) {
					navData = navSys->GetMainNavData(FNavigationSystem::DontCreate);
				}

				FNavLocation outResult;
				if (navSys->GetRandomReachablePointInRadius(orginLocation, radius, outResult, navData
									, UNavigationQueryFilter::GetQueryFilter(*navData, GetWorld(), GetDefaultNavigationFilterClass()))) 
				{
					GoalLocation = outResult.Location;
					return true;
				}
			}
		}
	}
	return false;
}

void AAIControllerSV::FindPathForMoveRequest(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const
{
	if (MoveRequest.IsMoveToActorRequest())
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(GetPawn());
		if (pawn && !pawn->IsMajor())
		{
			FVector goalRandomLocation = FVector::ZeroVector;
			if (GetRandomReachablePointInRadius(MoveRequest.GetGoalActor()->GetActorLocation(), goalRandomLocation)) {
				Query.EndLocation = goalRandomLocation;
			}
		}
	}
	
	Super::FindPathForMoveRequest(MoveRequest, Query, OutPath);
}

#define EXTENT_RADIUS 350
FPathFollowingRequestResult AAIControllerSV::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	ACharacterSV *pawn = Cast<ACharacterSV>(GetPawn());
	if (pawn)
	{
		if (AISpeedType != ESpeedType::None) {
			pawn->SetSpeedType(AISpeedType);
		}
		else
		{
			FAIExtra &extra = GetAIExtra();
			if (extra.SpeedTypes.Num() > 1) {
				pawn->SetSpeedType(extra.SpeedTypes[UKismetMathLibrary::RandomInteger(extra.SpeedTypes.Num())]);
			}
			else if (extra.SpeedTypes.Num() > 0) {
				pawn->SetSpeedType(extra.SpeedTypes[0]);
			}
			else {
				pawn->SetSpeedType(ESpeedType::Walk);
			}
		}

		UNavigationSystem* navSys = UNavigationSystem::GetCurrent(GetWorld());
		if (navSys)
		{
			const FNavAgentProperties& agentProps = GetNavAgentPropertiesRef();
			ANavigationData* navData = navSys->GetNavDataForProps(agentProps);
			if (navData == nullptr) {
				navData = navSys->GetMainNavData(FNavigationSystem::DontCreate);
			}

			FSharedConstNavQueryFilter queryFilter = UNavigationQueryFilter::GetQueryFilter(*navData, GetWorld(), GetDefaultNavigationFilterClass());

			FNavLocation ProjectedLocation;
			if (!navSys->ProjectPointToNavigation(pawn->GetActorLocation(), ProjectedLocation, INVALID_NAVEXTENT, &agentProps, queryFilter))
			{
				FVector goalLocation = FVector::ZeroVector;
				if (MoveRequest.IsMoveToActorRequest())
				{
					const ACharacterSV *goalPawn = Cast<ACharacterSV>(MoveRequest.GetGoalActor());
					if (goalPawn) {
						goalLocation = goalPawn->GetActorLocation();
					}
				}
				else {
					goalLocation = MoveRequest.GetGoalLocation();
				}

				ARecastNavMesh* recastNavData = Cast<ARecastNavMesh>(navData);
				if (recastNavData)
				{
					float extentRadius = EXTENT_RADIUS;
					const FGlobalConfigRow *config = &FGlobalConfigRes::Get().GetConfig("ExtentRadius");
					if (config && config->FloatValue.Num() > 0) {
						extentRadius = config->FloatValue[0];
					}

					FVector direction = goalLocation - pawn->GetActorLocation();
					FVector testLocation = pawn->GetActorLocation() + direction.GetSafeNormal() * extentRadius / 2;

					NavNodeRef ref = recastNavData->FindNearestPoly(testLocation, FVector(extentRadius, extentRadius, extentRadius * 2), queryFilter, this);

					FVector pointOnPoly;
					if (ref && recastNavData->GetClosestPointOnPoly(ref, testLocation, pointOnPoly))
					{
						FAIMoveRequest moveReq;
						if (MoveRequest.GetGoalActor()) {
							moveReq.SetGoalActor(MoveRequest.GetGoalActor());
						}
						else {
							moveReq.SetGoalLocation(pointOnPoly);
						}

						moveReq.SetAcceptanceRadius(MoveRequest.GetAcceptanceRadius());
						moveReq.SetReachTestIncludesAgentRadius(MoveRequest.IsReachTestIncludingAgentRadius());
						moveReq.SetAllowPartialPath(MoveRequest.IsUsingPartialPaths());
						moveReq.SetUsePathfinding(false);
						moveReq.SetNavigationFilter(MoveRequest.GetNavigationFilter());
						moveReq.SetCanStrafe(MoveRequest.CanStrafe());
						moveReq.SetProjectGoalLocation(MoveRequest.IsProjectingGoal());

						return Super::MoveTo(moveReq, OutPath);
					}
				}
			}
		}
	}

	return Super::MoveTo(MoveRequest, OutPath);
}

FAIRequestID AAIControllerSV::RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path)
{
	if (Path.IsValid()) 
	{
		Path->AddObserver(FNavigationPath::FPathObserverDelegate::FDelegate::CreateUObject(this, &AAIControllerSV::OnPathEvent));
		BuildSubPath(Path.Get());
	}

	return Super::RequestMove(MoveRequest, Path);
}

void AAIControllerSV::OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event)
{
	if (InPath)
	{
		switch (Event)
		{
		case ENavPathEvent::UpdatedDueToGoalMoved:
		case ENavPathEvent::UpdatedDueToNavigationChanged:
		case ENavPathEvent::MetaPathUpdate:
			{
				BuildSubPath(InPath);
			}
			break;

		default:
			break;
		}
	}
}

#define STEP_DIS 150
void AAIControllerSV::BuildSubPath(FNavigationPath* InPath)
{
	FAIExtra &extra =  GetAIExtra();
	if (!extra.bBuildSubPath) {
		return;
	}

	if (Blackboard)
	{
		if (Blackboard->GetValueAsEnum("State") == EStateType::Patrol) {
			return;
		}
	}

	int subRadius = extra.SubRadius;
	if (subRadius == 0)
		subRadius = STEP_DIS;

	TArray<FNavPathPoint> &pathPoints = InPath->GetPathPoints();
	TArray<FNavPathPoint> newPathPoints;
	
	UNavigationSystem* navSys = UNavigationSystem::GetCurrent(GetWorld());
	if (navSys)
	{
		ANavigationData* navData = navSys->GetNavDataForProps(GetNavAgentPropertiesRef());
		if (navData == nullptr) {
			navData = navSys->GetMainNavData(FNavigationSystem::DontCreate);
		}

		if (navData)
		{
			newPathPoints.Add(pathPoints[0]);
			//DrawDebugSphere(GetWorld(), pathPoints[0], 50, 10, FColor::Red, false, 10);
			for (int i = 1; i < pathPoints.Num(); ++i)
			{
				FVector dir = pathPoints[i].Location - pathPoints[i - 1].Location;
				int stepCount = dir.Size() / subRadius;
				if (stepCount <= 0)
					stepCount = 1;

				FVector offset = dir / (stepCount + 1);
				float radius = dir.Size() / (stepCount + 1);

				FVector origin = pathPoints[i - 1].Location;

				for (int j = 0; j < stepCount; ++j)
				{
					origin += offset;
					FNavLocation outResult;
					if (navSys->GetRandomReachablePointInRadius(origin, radius, outResult, navData, UNavigationQueryFilter::GetQueryFilter(*navData, GetWorld(), DefaultNavigationFilterClass))) {
						newPathPoints.Add(FNavPathPoint(outResult));
					}
					//DrawDebugSphere(GetWorld(), outResult, 50, 10, FColor::Green, false, 10);
				}
				//DrawDebugSphere(GetWorld(), pathPoints[i], 50, 10, FColor::Red, false, 10);
				newPathPoints.Add(pathPoints[i]);
			}
			pathPoints = newPathPoints;
		}
	}
}

void AAIControllerSV::BeginPlay()
{
	Super::BeginPlay();
	bSeamlessTravel = true;
}

void AAIControllerSV::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	ACharacterSV* ch = Cast<ACharacterSV>(GetPawn());
	if (ch)
	{
		return ch->SetGenericTeamId(NewTeamID);
	}
}

FGenericTeamId AAIControllerSV::GetGenericTeamId() const
{
	ACharacterSV* ch = Cast<ACharacterSV>(GetPawn());
	if (ch)
	{
		return ch->GetGenericTeamId();
	}

	return FGenericTeamId();
}

ETeamAttitude::Type AAIControllerSV::GetTeamAttitudeTowards(const AActor& Other) const
{
	ACharacterSV* ch = Cast<ACharacterSV>(GetPawn());
	if (ch)
	{
		return ch->GetTeamAttitudeTowards(Other);
	}

	return ETeamAttitude::Neutral;
}