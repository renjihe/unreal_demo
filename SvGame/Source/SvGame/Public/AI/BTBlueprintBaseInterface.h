// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Common/AIData.h"
#include "BTBlueprintBaseInterface.generated.h"

class ACharacterSV;

UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class SVGAME_API UBTBlueprintBaseInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class SVGAME_API IBTBlueprintBaseInterface
{
	GENERATED_IINTERFACE_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual UObject* GetValueAsObject(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual UClass* GetValueAsClass(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual uint8 GetValueAsEnum(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual int32 GetValueAsInt(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual float GetValueAsFloat(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual bool GetValueAsBool(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual FString GetValueAsString(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual FName GetValueAsName(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual FVector GetValueAsVector(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual FRotator GetValueAsRotator(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual ACharacterSV* GetValueAsActor(const FString& KeyName) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsObject(const FString& KeyName, UObject* ObjectValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsClass(const FString& KeyName, UClass* ClassValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsEnum(const FString& KeyName, uint8 EnumValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsInt(const FString& KeyName, int32 IntValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsFloat(const FString& KeyName, float FloatValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsBool(const FString& KeyName, bool BoolValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsString(const FString& KeyName, FString StringValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsName(const FString& KeyName, FName NameValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsVector(const FString& KeyName, FVector VectorValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom|Blackboard")
	virtual void SetValueAsRotator(const FString& KeyName, FRotator VectorValue, bool bSendQueuedObserverNotifications);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool GetRandomReachablePointInRadius(bool bOrginLocation, float Radius, FVector &GoalLocation) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual int GetAbilityIDByIndex(EAbilityBar::Type index) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool GetPatrolPoint(FVector &GoalLocation) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void CompletePatrolMove();

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool GetMasterPathPoint(FVector &GoalLocation) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void CompleteMasterPathMove();

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool TestMasterPathFinish() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool SelectNeedRescueMajor(ACharacterSV *&Target) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool SelectNeedCureMajor(float HealthRate, ACharacterSV *&Target) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void ChangeAIState(EStateType::Type State, bool bSendQueuedObserverNotifications = false);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void SetTarget(ACharacterSV *Target, bool bChangeState = false, bool bSendQueuedObserverNotifications = false);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual ACharacterSV *GetMajorNearest(bool bBot, int Radius = 0) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual float CalcDistance(AActor *Actor) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual ECharacterRole::Type GetCharRole() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual bool TestTraceTarget(ACharacterSV *Target) const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void FaceTarget(ACharacterSV *Target);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual void AimTarget(ACharacterSV *Target);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual FAIExtra& GetAIExtra() const;

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	virtual ACharacterSV *ChooseTarget(EChooseTarget::Type ChooseType, ETeamAttitude::Type TeamAttitude, bool bWarnRadius) const;

protected:
	AAIController* BtAIOwner;
};
