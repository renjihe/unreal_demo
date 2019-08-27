

#include "SvGame.h"

#include "GameModeSVBattle.h"
#include "TargetPointSV.h"
#include "AIPerceptionComponentSV.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTBlueprintBaseInterface.h"

#include "AIControllerSV.h"


UBTBlueprintBaseInterface::UBTBlueprintBaseInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


#define HANDLE_BLACKBOARD() \
	UBlackboardComponent *Blackboard = nullptr; \
	AAIControllerSV *aiController = Cast<AAIControllerSV>(BtAIOwner); \
	if (aiController) { \
		Blackboard = aiController->GetBlackboard(); \
	}\

#define HANDLE_PERCEPTION() \
	UAIPerceptionComponentSV *PerceptionComponent = nullptr; \
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner); \
	if (controller) { \
		PerceptionComponent = Cast<UAIPerceptionComponentSV>(controller->GetPerceptionComponent()); \
	}\

UObject* IBTBlueprintBaseInterface::GetValueAsObject(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsObject(FName(*KeyName));
	}
	return NULL;
}

UClass* IBTBlueprintBaseInterface::GetValueAsClass(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsClass(FName(*KeyName));
	}
	return NULL;
}

uint8 IBTBlueprintBaseInterface::GetValueAsEnum(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsEnum(FName(*KeyName));
	}
	return 0;
}

int32 IBTBlueprintBaseInterface::GetValueAsInt(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsInt(FName(*KeyName));
	}
	return 0;
}

float IBTBlueprintBaseInterface::GetValueAsFloat(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsFloat(FName(*KeyName));
	}
	return 0.0f;
}

bool IBTBlueprintBaseInterface::GetValueAsBool(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsBool(FName(*KeyName));
	}
	return false;
}

FString IBTBlueprintBaseInterface::GetValueAsString(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsString(FName(*KeyName));
	}
	return FString();
}

FName IBTBlueprintBaseInterface::GetValueAsName(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsName(FName(*KeyName));
	}
	return FName();
}

FVector IBTBlueprintBaseInterface::GetValueAsVector(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsVector(FName(*KeyName));
	}
	return FVector();
}

FRotator IBTBlueprintBaseInterface::GetValueAsRotator(const FString& KeyName) const
{
	HANDLE_BLACKBOARD()

	if (Blackboard) {
		return Blackboard->GetValueAsRotator(FName(*KeyName));
	}
	return FRotator();
}

ACharacterSV* IBTBlueprintBaseInterface::GetValueAsActor(const FString& KeyName) const
{
	return Cast<ACharacterSV>(GetValueAsObject(KeyName));
}

void IBTBlueprintBaseInterface::SetValueAsObject(const FString& KeyName, UObject* ObjectValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard) 
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsObject(FName(*KeyName), ObjectValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsClass(const FString& KeyName, UClass* ClassValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsClass(FName(*KeyName), ClassValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsEnum(const FString& KeyName, uint8 EnumValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsEnum(FName(*KeyName), EnumValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsInt(const FString& KeyName, int32 IntValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsInt(FName(*KeyName), IntValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsFloat(const FString& KeyName, float FloatValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsFloat(FName(*KeyName), FloatValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsBool(const FString& KeyName, bool BoolValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsBool(FName(*KeyName), BoolValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsString(const FString& KeyName, FString StringValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsString(FName(*KeyName), StringValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsName(const FString& KeyName, FName NameValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsName(FName(*KeyName), NameValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsVector(const FString& KeyName, FVector VectorValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsVector(FName(*KeyName), VectorValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetValueAsRotator(const FString& KeyName, FRotator VectorValue, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		Blackboard->SetValueAsRotator(FName(*KeyName), VectorValue);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

ACharacterSV *IBTBlueprintBaseInterface::GetMajorNearest(bool bBot, int Radius) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) {
		return controller->GetMajorNearest(bBot, Radius);
	}

	return NULL;
}

bool IBTBlueprintBaseInterface::GetRandomReachablePointInRadius(bool bOrginLocation, float Radius, FVector &GoalLocation) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) 
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn) {
			return controller->GetRandomReachablePointInRadius(bOrginLocation ? pawn->OrginLocation : pawn->GetActorLocation(), GoalLocation, Radius);
		}
	}

	return false;
}

int IBTBlueprintBaseInterface::GetAbilityIDByIndex(EAbilityBar::Type index) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn) {
			return pawn->GetAbilityIDByIndex(index);
		}
	}
	return 0;
}

bool IBTBlueprintBaseInterface::GetPatrolPoint(FVector &GoalLocation) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		if (controller->TargetPoints.Num() > 1)
		{
			if (controller->CurrTargetPoint >= controller->TargetPoints.Num()) {
				controller->CurrTargetPoint = 0;
			}
			GoalLocation = controller->TargetPoints[controller->CurrTargetPoint]->GetActorLocation();
			return true;
		}

		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn) 
		{
			FAIExtra &AIExtra = controller->GetAIExtra();
			return controller->GetRandomReachablePointInRadius(pawn->OrginLocation, GoalLocation, AIExtra.PatrolRadius);
		}
	}

	return false;
}

void IBTBlueprintBaseInterface::CompletePatrolMove()
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		if (controller->TargetPoints.Num() > 1)
		{
			controller->CurrTargetPoint++;
			if (controller->CurrTargetPoint >= controller->TargetPoints.Num()) {
				controller->CurrTargetPoint = 0;
			}
		}
	}
}


bool IBTBlueprintBaseInterface::GetMasterPathPoint(FVector &GoalLocation) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(controller->GetWorld()->GetAuthGameMode());
		if (GameMode && GameMode->MasterPathPoints.Num() > 0)
		{
			//update CurrMasterPathIndex
			ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
			if (pawn && !pawn->bInSafeHouse)
			{
				float nearestDis = FLT_MAX;
				int nearestIndex = 0;
				FVector location = pawn->GetActorLocation();

				for (int i = 0; i < GameMode->MasterPathPoints.Num(); ++i)
				{
					FVector dis = GameMode->MasterPathPoints[i] - location;
					if (dis.Size() < nearestDis)
					{
						nearestDis = dis.Size();
						nearestIndex = i;
					}
				}

				if (nearestIndex == GameMode->MasterPathPoints.Num() - 1) {
					controller->CurrMasterPathIndex = nearestIndex;
				}
				else if (nearestDis < 100) {
					controller->CurrMasterPathIndex = nearestIndex + 1;
				}
				else
				{
					FVector dirA = GameMode->MasterPathPoints[nearestIndex] - location;
					FVector dirB = GameMode->MasterPathPoints[nearestIndex] - GameMode->MasterPathPoints[nearestIndex + 1];

					if (dirA.X * dirB.X + dirA.Y * dirB.Y > 0) {//<90 
						controller->CurrMasterPathIndex = nearestIndex + 1;
					}
					else {
						controller->CurrMasterPathIndex = nearestIndex;
					}
				}
			}

			//end
			if (controller->CurrMasterPathIndex >= 0 && controller->CurrMasterPathIndex < GameMode->MasterPathPoints.Num())
			{
				GoalLocation = GameMode->MasterPathPoints[controller->CurrMasterPathIndex];
				return true;
			}
		}
	}
	return false;
}

void IBTBlueprintBaseInterface::CompleteMasterPathMove()
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) {
		controller->CurrMasterPathIndex++;
	}
}

bool IBTBlueprintBaseInterface::TestMasterPathFinish() const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) 
	{
		AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(controller->GetWorld()->GetAuthGameMode());
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (GameMode && pawn) {
			return controller->CurrMasterPathIndex >= GameMode->MasterPathPoints.Num() && pawn->bInSafeHouse;
		}
	}
	return true;
}

bool IBTBlueprintBaseInterface::SelectNeedRescueMajor(ACharacterSV *&Target) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(controller->GetWorld()->GetAuthGameMode());
		if (GameMode) 
		{
			for (auto It = GameMode->CreatePawnConstIterator(); It; ++It)
			{
				ACharacterSV *OtherPawn = Cast<ACharacterSV>(*It);
				if (OtherPawn && OtherPawn->GetAttrib(EAttribType::Extremis) > 0)
				{
					Target = OtherPawn;
					return true;
				}
			}
		}
	}
	return false;
}

bool IBTBlueprintBaseInterface::SelectNeedCureMajor(float HealthRate, ACharacterSV *&Target) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller)
	{
		AGameModeSVBattle *GameMode = Cast<AGameModeSVBattle>(controller->GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			for (auto It = GameMode->CreatePawnConstIterator(); It; ++It)
			{
				ACharacterSV *OtherPawn = Cast<ACharacterSV>(*It);
				if (OtherPawn && OtherPawn->GetAttrib(EAttribType::Health) / OtherPawn->GetAttrib(EAttribType::MaxHealth) < HealthRate)
				{
					Target = OtherPawn;
					return true;
				}
			}
		}
	}
	return false;
}

void IBTBlueprintBaseInterface::ChangeAIState(EStateType::Type State, bool bSendQueuedObserverNotifications)
{
	HANDLE_BLACKBOARD()

	if (Blackboard)
	{
		Blackboard->PauseObserverNotifications();
		aiController->ChangeAIState(State);
		Blackboard->ResumeObserverNotifications(bSendQueuedObserverNotifications);
	}
}

void IBTBlueprintBaseInterface::SetTarget(ACharacterSV *Target, bool bChangeState, bool bSendQueuedObserverNotifications)
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) {
		controller->SetTarget(Target, bChangeState, bSendQueuedObserverNotifications);
	}
}

float IBTBlueprintBaseInterface::CalcDistance(AActor *Actor) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (Actor && controller && controller->GetPawn()) {
		return (Actor->GetActorLocation() - controller->GetPawn()->GetActorLocation()).Size();
	}
	return 0;
}

ECharacterRole::Type IBTBlueprintBaseInterface::GetCharRole() const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) 
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn) {
			return pawn->GetCharRole();
		}
	}
	return ECharacterRole::MOB;
}

bool IBTBlueprintBaseInterface::TestTraceTarget(ACharacterSV *Target) const
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (Target && controller)
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn)
		{
			FCollisionQueryParams collisionQueryParams("AITestTraceTarget", true, Target);
			collisionQueryParams.AddIgnoredActor(pawn);

			ECollisionChannel DefaultSightCollisionChannel = UEngineTypes::ConvertToCollisionChannel(TRACE_FIRE);//GET_AI_CONFIG_VAR(DefaultSightCollisionChannel);
			FHitResult HitResult;
			const bool bHit = controller->GetWorld()->LineTraceSingleByChannel(HitResult, pawn->GetActorLocation(), Target->GetActorLocation()
				, DefaultSightCollisionChannel
				, collisionQueryParams);

			if (bHit == true && !HitResult.Actor->IsOwnedBy(Target) && !HitResult.Actor->IsOwnedBy(pawn)) {
				return false;
			}
		}
	}

	return true;
}

void IBTBlueprintBaseInterface::FaceTarget(ACharacterSV *Target)
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (Target && controller) 
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn)
		{
			//Target->RootComponent->GetComponentLocation() - pawn->RootComponent->GetComponentLocation()
			FRotator rotator = FRotationMatrix::MakeFromX(Target->GetActorLocation() - pawn->GetActorLocation()).Rotator();
			rotator.Pitch = rotator.Roll = 0;
			pawn->SetActorRotation(rotator);
		}
	}
}

void IBTBlueprintBaseInterface::AimTarget(ACharacterSV *Target)
{
	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (Target && controller)
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn)
		{
			FName boneName;
			pawn->AimTarget(Target, boneName);
		}
	}
}

FAIExtra& IBTBlueprintBaseInterface::GetAIExtra() const
{
	static FAIExtra defaultAIExtra;

	AAIControllerSV *controller = Cast<AAIControllerSV>(BtAIOwner);
	if (controller) {
		return controller->GetAIExtra();
	}
	return defaultAIExtra;
}

ACharacterSV *IBTBlueprintBaseInterface::ChooseTarget(EChooseTarget::Type ChooseType, ETeamAttitude::Type TeamAttitude, bool bWarnRadius) const
{
	HANDLE_PERCEPTION()

	ACharacterSV *ret = nullptr;
	bool bDeadDataFound = false;

	if (PerceptionComponent)
	{
		ACharacterSV *pawn = Cast<ACharacterSV>(controller->GetPawn());
		if (pawn)
		{
			int radius = bWarnRadius ? pawn->AIExtra.WarnRadius : 0;

			FVector location = pawn->GetActorLocation();

			float nearestDis = FLT_MAX;
			for (auto It = PerceptionComponent->GetPerceptualDataConstIterator(); It; ++It)
			{
				if (It->Value.Target.IsValid())
				{
					if ((TeamAttitude == ETeamAttitude::Hostile) == It->Value.bIsHostile)
					{
						ACharacterSV *otherPawn = Cast<ACharacterSV>(It->Value.Target.Get());
						if (otherPawn && pawn != otherPawn)
						{
							if (otherPawn->IsMinion() || otherPawn->IsMinionAttract())
								continue;

							if (otherPawn->IsDead() || !It->Value.HasAnyCurrentStimulus())
								continue;

							FVector dis = otherPawn->GetActorLocation() - location;
							if (dis.Size() < nearestDis && (0 == radius || dis.Size() < radius))
							{
								nearestDis = dis.Size();
								ret = Cast<ACharacterSV>(otherPawn);
							}
						}
					}
				}
				else {
					bDeadDataFound = true;
				}
			}
		}
	}
	
	if (bDeadDataFound) {
		PerceptionComponent->RemoveDeadData();
	}

	return ret;
}
