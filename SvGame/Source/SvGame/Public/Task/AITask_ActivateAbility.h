// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "Tasks/AITask.h"
#include "GameplayTaskResource.h"
#include "AITask_ActivateAbility.generated.h"

class AAIController;

UCLASS(meta = (DisplayName = "Activate Ability"))
class SVGAME_API UAIResource_ActivateAbility : public UGameplayTaskResource
{
	GENERATED_BODY()
};

namespace EAbilityTaskResult
{
	enum Type
	{
		Failed,
		Success,
		Finished,
	};
}


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAbilityTaskCompletedSignature, int, AbilityId, AAIController*, AIController);

struct AbilityRequest
{
	APawn* Pawn;
	AActor* GoalActor;
	FVector GoalLocation;
	int AbilityId;
};

UCLASS()
class SVGAME_API UAITask_ActivateAbility : public UAITask
{
	GENERATED_BODY()

public:
	UAITask_ActivateAbility(const FObjectInitializer& ObjectInitializer);

	void SetUpCallBack(bool bBind = false);
	void SetUp(AAIController* Controller);

	UFUNCTION(BlueprintCallable, Category = "AI|Tasks", meta = (AdvancedDisplay = "AbilityIndex", DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE", DisplayName = "Activate Ability"))
	static UAITask_ActivateAbility* ActivateAbility(AAIController* Controller, APawn* Pawn, FVector GoalLocation, AActor* GoalActor = nullptr, int AbilityId = 0, bool bLockAILogic = false);

	void OnRequestFinishedCallBack(int AbilityId);
	void OnRequestSuccessCallBack(int AbilityId);

	AbilityRequest AbilityReq;
protected:
	
	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnRequestFailed;

	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnRequestSuccess;

	UPROPERTY(BlueprintAssignable)
	FAbilityTaskCompletedSignature OnRequestFinished;

	FDelegateHandle RequestFinishedDelegateHandle;
	FDelegateHandle RequestSuccessDelegateHandle;

	void FinishMoveTask(EAbilityTaskResult::Type InResult, int AbilityId = 0);

	virtual void Activate() override;
	virtual void OnDestroy(bool bOwnerFinished) override;

	virtual void Pause() override;
	virtual void Resume() override;
};
