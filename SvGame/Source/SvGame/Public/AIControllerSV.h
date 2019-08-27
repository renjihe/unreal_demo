// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/AIData.h"
#include "AbilitiesType.h"

#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BTTaskNode.h"

#include "AIControllerSV.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class ACharacterSV;
class UAISenseConfig;
class ATargetPointSV;
/**
 * 
 */
UCLASS()
class SVGAME_API AAIControllerSV : public AAIController
{
	GENERATED_BODY()

public: //interface
	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	ACharacterSV *GetMajorNearest(bool bBot, int Radius = 0);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	FAIExtra& GetAIExtra();

public:
	AAIControllerSV(const FObjectInitializer& ObjectInitializer);

	UBlackboardComponent* GetBlackboard() { return Blackboard;  };
	virtual void Possess(APawn* InPawn) override;
	virtual void UnPossess() override;

	virtual bool RunBehaviorTreeByID(int BTId);

	void UpdateBlackboardAttribValue(EAttribType::Type KeyID);
	void ChangeAIState(EStateType::Type State);

	bool GetRandomReachablePointInRadius(const FVector &OrginLocation, FVector &GoalLocation, float Radius = -1) const;
	virtual void FindPathForMoveRequest(const FAIMoveRequest& MoveRequest, FPathFindingQuery& Query, FNavPathSharedPtr& OutPath) const;
	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr) override;
	virtual FAIRequestID RequestMove(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr Path) override;
	void OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event);
	void BuildSubPath(FNavigationPath* InPath);

	void SetTarget(ACharacterSV *Target, bool bChangeState = false, bool bSendQueuedObserverNotifications = false);
	ACharacterSV *GetTarget();

	UFUNCTION()
	void OnPerceptionUpdated(TArray<AActor*> UpdatedActors);
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	void OnTargetDeadCallBack(ACharacterSV *Target);

	bool CheckStimulusType(FAISenseID Type);

	void Init();
	virtual void InitTargetPoint();
	virtual void BeginPlay() override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
public:
	bool bSeamlessTravel;

	TEnumAsByte<ESpeedType::Type> AISpeedType;

	TArray<ATargetPointSV*> TargetPoints;
	int CurrTargetPoint;

	int CurrMasterPathIndex;

private:
	//TArray<UAISenseConfig*> SenseConfig;
	FDelegateHandle TargetDeadDelegateHandle;

	FAIExtra DefaultAIExtra;
	void InitializeBlackboardData();

	void OnStateOut();
	void OnStateEnter();
};
