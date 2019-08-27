// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Weapon/BaseWeapon.h"
#include "Common/GameData.h"
#include "Common/AIData.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"

// -------------------------------------------------------------------------------------------
//			GameplayAbility System Sample
// -------------------------------------------------------------------------------------------
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/AbilitiesType.h"
#include "Res/AbilityInfoRes.h"
//#include "Res/HitTestActorRes.h"
#include "Res/JobRes.h"
#include "Res/LookRes.h"
#include "Res/ShakeRes.h"
#include "Res/SpreadRes.h"
#include "Res/BufRes.h"
#include "CharacterSVMovementComponent.h"
#include "AnimInstanceBase.h"

#include "CharacterSV.generated.h"

class AHitTestActor;
class AHitImpactEffect;
class AAbilityEntity;
class ACharacterSV;
class ADismemberActor;

class USplineMeshComponent;

#define BUFFUNC_COUNT 16
USTRUCT()
struct FBuff
{
	GENERATED_USTRUCT_BODY()

	const FBufRow	*Info;
	UPROPERTY()
	int				BuffID;
	UPROPERTY()
	int				SerialNumber;

	UPROPERTY()
	int				FrameCounter;
	UPROPERTY()
	int				EffectID;

	UPROPERTY()
	unsigned		PeriodicCounter;
	UPROPERTY()
	unsigned		BuffPeriodicFrames;
	UPROPERTY()
	FAttribs		CasterAttrib;

	int				UserData[BUFFUNC_COUNT];
	void			*UserDataPtr[BUFFUNC_COUNT];
};

#define NoTeamId 255

DECLARE_MULTICAST_DELEGATE_OneParam(FActivateAbilitySignature, int);
DECLARE_MULTICAST_DELEGATE_OneParam(FCharacterDeadDelegate, ACharacterSV*);

UCLASS()
class SVGAME_API ACharacterSV : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UPhysicalAnimationComponent *PhysicalAnimation;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USplineComponent* SplineComp;
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController *newController) override;
	virtual void OnRep_Controller() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//virtual FVector GetMoveGoalOffset(const AActor* MovingActor) const override;

	UFUNCTION(BlueprintCallable)
	void Init(FVector Location, FRotator Rotator);
	void UpdateLook();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Component")
	virtual UMeshComponent* GetSpecMesh(ESpecMesh::Type type);

	UFUNCTION(BlueprintCallable, Category = "Component")
	virtual AActor* GetSpecActor(ESpecMesh::Type type);

	UFUNCTION(BlueprintCallable, Category = "Component")
	UMeshComponent* GetSpecMeshByPosition(EEffectPosition::Type position);
	//Team
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TEnumAsByte<ESpeedType::Type> SpeedType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	uint8 TeamID;

	UFUNCTION(BlueprintPure)
	int GetTeamID();

	void SetSpeedType(ESpeedType::Type Type);
	int CalcSpeedAttrib();

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UFUNCTION(BlueprintPure)
	static ETeamAttitude::Type GetTeamAttitudeTowards(AActor* target1, AActor* target2);

	UFUNCTION(BlueprintPure)
	static ETeamAttitude::Type GetTeamAttitudeTowardsByTeamID(int targetTeamID, AActor* target);

	//buf
private:
	UPROPERTY(ReplicatedUsing = OnRep_BuffData)
	TArray<FBuff> BuffData;

	UPROPERTY(ReplicatedUsing = OnRep_DebuffData)
	TArray<FBuff> DebuffData;

	TArray<FBuff> ClientBuffData;
	TArray<FBuff> ClientDebuffData;

	bool bDirtyClientBuff;
	bool bDirtyClientDebuff;
public:
	void TickBuff();
	void AddBuff(int BuffID, int Frames, int BuffPeriodicFrames, FAttribs CasterAttrib);
	void RemoveBuff(int BuffID);

	void ActiveBuff(FBuff &Buff, bool bActive, bool bClent = false);
	void DoBuff(const FBufRow *Info, bool bActive, FBuff *Buff = NULL, bool bClent = false);
	FBuff *FindBuffByID(int BuffID, bool bClient = false);
	FBuff *FindBuffBySerial(int SerialNumber, bool bClient = false);

	UFUNCTION()
	void OnRep_DebuffData();
	UFUNCTION()
	void OnRep_BuffData();

	void OnRep_DoBuff(bool bDebuf);
	//attribute
public:
	UFUNCTION(BlueprintPure, Category = "Attrib")
	int GetAttrib(EAttribType::Type attribType);

	UFUNCTION(BlueprintPure, Category = "Attrib")
	FAttribs GetAttribs();

	UFUNCTION(BlueprintCallable, Category = "Attrib")
	void SetAttrib(EAttribType::Type attribType, int value);

	UFUNCTION(BlueprintCallable, Category = "Attrib")
	void AddAttrib(EAttribType::Type attribType, int value);

	static CharacterBodyPart::Type GetBodyPart(EPhysicalSurface Surface);

	static float CalcProjectileDamage(float damage, FAttribs attackAttrib, FAttribs targetAttrib, AAbilityEntity* entity, EPhysicalSurface surface, float Distance);
	static float CalcMeleeDamage(float damage, FAttribs attackAttrib, FAttribs targetAttrib);
	
	//Abilities
public:
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

	UPROPERTY(Replicated)
	TArray<int32> AbilityList;

	UPROPERTY(Replicated)
	TArray<FAbilityState> AbilityStateList;

	UPROPERTY(Replicated)
	TArray<bool> AbilityBarState;

	//bool TouchOffState[EAbilityBar::Count];

	bool MontageStop;
	UAnimMontage* MontageToPlay;

	bool GripMontageStop;
	UAnimMontage* GripMontageToplay;
	
	//是否拥有此技能
	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool IsExistAbility(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	int GetAbilityIndex(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FAbilityInfoRow GetAbilityInfoByID(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FAbilityInfoRow GetAbilityInfoByIndex(int index) const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	int GetAbilityIDByIndex(EAbilityBar::Type index) const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	int SelectAttackAbility() const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	int SelectCureAbility() const;

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	int GetAbilityStateIndex(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FAbilityState GetAbilityStateByID(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FAbilityState GetAbilityStateByIndex(int index) const;

	//是否正在使用技能
	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool IsUsedAbility();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool IsIndexAbilityUsing(EAbilityBar::Type type);

	void InitAbility();

	void UpdateAbility(float DeltaTime);

	//UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	//void AddAbilitys(TMap<int, int> abilitys, AActor* abilityEntity, USceneComponent* abilityEntityComp = nullptr, int parentAbilityID = 0);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AddAbility(EAbilityBar::Type index ,int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool TryActivateAbilityByIndex(int index, AActor* target = nullptr, bool allowRemoteActivation = true);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool TryActivateAbilityByID(int abilityID, AActor* target = nullptr, bool allowRemoteActivation  = true);

	//UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	//void ClearAbilitys(TMap<int, int> abilitys);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void ClearAbilityByID(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void ClearAbilityByIndex(int32 index);

	UFUNCTION()
	static void AbilityActivatedCallBack(UGameplayAbility* ability);

	//删除
	//UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AbilityActivated(int abilityID);

	UFUNCTION()
	static void AbilityEndedCallback(UGameplayAbility* ability);

	//删除
	//UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	virtual void AbilityEnded(int abilityID);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AbilityTouchOff(EAbilityBar::Type index, bool touchOff, AActor* target = nullptr, bool cancelAbility = false);

	UFUNCTION(reliable, Server, WithValidation)
	void ServeAbilityTouchOff(EAbilityBar::Type index, bool touchOff, AActor* target, bool cancelAbility = false);

	void EndOrCancelAbilityByID(int abilityID, bool bWantCancel);
	void EndOrCancelAbilityByIndex(int index, bool bWantCancel);
	
	//技能接口
public:
	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	static void GetFromActorAndMesh(AActor *fromActor, EEffectPosition::Type fromPosition, AActor *&actor, UMeshComponent *&meshComp);

	static FTransform MakeFromTransform(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relaLocation = FVector::ZeroVector, FRotator relaRotation = FRotator::ZeroRotator);
	static UParticleSystemComponent* PlayParticleInternal(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation, bool bAutoDestroy = true);
	static UAudioComponent* PlaySoundInternal(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation);

	UFUNCTION(BlueprintPure, Category = "Ability|Custom")
	static FVector GetTargetLocationAndBone(AActor* target, FName &boneName);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom", meta = (DisplayName = "AimTarget", Keywords = "Aim Target"))
	void K2_AimTarget(AActor* target, FName &boneName);

	virtual void AimTarget(AActor* target, FName &boneName);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	bool NeedReload();

	//Damage
protected:
	FName ImpulseBone;
	float ImpulseTimer;

public:

	//UFUNCTION(BlueprintCallable, Category = "Damage")
	//void OnHit(FHitResult hit)

	UFUNCTION(NetMulticast, Reliable, Category = "Ability|Custom")
	void NetMulticastNotifyOnHit(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType = EAttackType::UnDefined);

	UFUNCTION(NetMulticast, Reliable, Category = "Ability|Custom")
	void NetMulticastNotifyDead(const FHitResult &hit, EAttackType::Type attackType = EAttackType::UnDefined);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Custom", meta = (DisplayName = "NotifyOnHit"))
	void K2_NotifyOnHit(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType = EAttackType::UnDefined);

	virtual void NotifyOnHit(const FHitResult &hit, int damageValue, int CurHealth, EAttackType::Type attackType = EAttackType::UnDefined);

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Custom", meta = (DisplayName = "NotifyDead"))
	void K2_NotifyDead(const FHitResult &hit, EAttackType::Type attackType = EAttackType::UnDefined);
	
	virtual void NotifyDead(const FHitResult &hit, EAttackType::Type attackType = EAttackType::UnDefined);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void Dismember(const FName &boneName, const FVector &impulse, float damage);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void AddImpulse(const FVector &impulse, const FName &boneName, const FVector &location);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastPlayParticle(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastPlayParticleTrail(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation
		, FName toParamName, const FVector &toLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastPlaySound(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastSpawnImpactEffects(const FHitResult& hit, TSubclassOf<AHitImpactEffect> impactEffect);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastCalcShakeOffset(AActor *fromActor, EEffectPosition::Type fromPosition);

	//UFUNCTION(NetMulticast, Unreliable)
	//void NetMulticastPlayMontage(USkeletalMeshComponent* skeletalMesh, UAnimMontage* montage, float rate = 1.f, FName startSection = NAME_None);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastPlayMontage(USkeletalMeshComponent* skeletalMesh, EAnimAction::Type ActionType, EEffectPosition::Type fromPosition, UAbilityTask_PlayMontageAndWaitSV* ability);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulticastStopMontage(USkeletalMeshComponent* skeletalMesh, EEffectPosition::Type fromPosition);

	//AI
public:
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (DisplayName = "GetAITarget"))
	ACharacterSV *GetAITarget() const;
public:

	//UFUNCTION(NetMulticast, Unreliable)
	//void NetMulticastPlayTrailParticle(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, const FVector &relativeLocation, const FRotator &relativeRotation);

	UPROPERTY(BlueprintReadOnly)
	bool bDismemberedArmL;
	UPROPERTY(BlueprintReadOnly)
	bool bDismemberedArmR;
	UPROPERTY(BlueprintReadOnly)
	bool bDismemberedHead;
	UPROPERTY(BlueprintReadOnly)
	bool bDismemberedLegL;
	UPROPERTY(BlueprintReadOnly)
	bool bDismemberedLegR;

	UPROPERTY(BlueprintReadWrite)
	bool bRagdoll;

	UPROPERTY(BlueprintReadOnly)
	bool bFallingCollisionMode;

	UPROPERTY(BlueprintReadOnly)
	bool bNoCollisionMode;

	UPROPERTY(BlueprintReadOnly)
	bool bInSafeHouse;

	TArray<int> DismemberHealth;
	TArray<ADismemberActor*> Dismembers;

	void TickPhysics(float deltaTime);
	int32 GetCurLookId()const {
		return CurrLookId;
	}
	void SetLook(int lookId);

	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	bool IsMajor() const { return CharRole == ECharacterRole::MAJOR;  }
	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	bool IsMob() const { return CharRole == ECharacterRole::MOB; }
	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	bool IsMinion() const { return CharRole == ECharacterRole::MINION; }
	UFUNCTION(BlueprintCallable, Category = "AI|Custom")
	bool IsMinionAttract() const { return CharRole == ECharacterRole::MINION_ATTRACT; }

	void SetCharRole(ECharacterRole::Type CRole) { CharRole = CRole; }
	ECharacterRole::Type GetCharRole() { return CharRole; }
	bool IsDead() { return bDead; }
	void SetDead(bool Dead);

	int OrginLookId;
	FVector OrginLocation;
	FRotator OrginRotator;

	UPROPERTY(EditAnywhere, Replicated)
	int LookId;

	UPROPERTY(EditAnywhere)
	bool DisableImpulse;

	UPROPERTY(EditAnywhere)
	bool DisableDismember;

	UPROPERTY(EditAnywhere, Replicated)
	int JobId;

	UPROPERTY(EditAnywhere)
	FAIExtra AIExtra;

	FActivateAbilitySignature OnActivateAbilitySuccess;
	FActivateAbilitySignature OnActivateAbilityFinished;

	FCharacterDeadDelegate OnCharacterDead;
protected:
	int32 CurrLookId;

	ECharacterRole::Type CharRole;
	bool  bDead;

	// Movement
protected:
	UPROPERTY(BlueprintReadOnly, Replicated, meta = (AllowPrivateAccess = "true"))
	float MoveSpeedTarget;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float MoveSpeedCurrent;

	void TickMoveSpeed(float deltaTime);

	//Spline
	bool bTraceSpline;

	bool bShowSplineMesh;

	UPROPERTY(EditDefaultsOnly, Category = Ability)
	float Velocity;

	UPROPERTY(BlueprintReadOnly, Category = Ability)
	FVector LaunchVelocity;

	UPROPERTY(BlueprintReadWrite, Category = Ability)
	TArray<USplineMeshComponent*> SplineMeshs;
	
	UPROPERTY(EditDefaultsOnly, Category = Ability)
	UStaticMesh* SplineStaticMesh;

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void ActiveSpline(bool active, float newVelocity = 0.0f, bool showMesh = false);

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void ClearSplinePath();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	void UpdateSplinePath();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FVector GetSplineLaunchVelocity();
};
