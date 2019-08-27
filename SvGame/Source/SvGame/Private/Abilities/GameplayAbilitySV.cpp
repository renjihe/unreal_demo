// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "GameplayAbilitySV.h"
#include "CharacterSV.h"
#include "ParticleRes.h"
#include "GlobalConfigRes.h"
#include "HitTestActor.h"
#include "AbilityEntity.h"
#include "Perception/AISense_Hearing.h"

#include "Perception/AISenseConfig_Damage.h"
#include "Abilities/AbilityProjectile.h"

UGameplayAbilitySV::UGameplayAbilitySV(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGameplayAbilitySV::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	HitTargets.Empty();

	for (auto i = HitTestActors.CreateIterator(); i; ++i) {
		GetWorld()->DestroyActor(*i);
	}
	HitTestActors.Empty();
	Target = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbilitySV::SetAbilityInfo(int newAbilityID, FAbilityInfoRow newAbilityInfo, FAttribs newAttribs, int newTeam, int newAbilityIndex)
{
	AbilityID = newAbilityID;
	AbilityInfo = newAbilityInfo;
	Attribs = newAttribs;
	TeamID = newTeam;
	AbilityIndex = newAbilityIndex;
}

void UGameplayAbilitySV::SetAttribs(FAttribs ownerAttribs)
{
	Attribs = ownerAttribs;
}

int UGameplayAbilitySV::GetAbilityID()
{
	return AbilityID;
}

void UGameplayAbilitySV::SetTeam(int newTeamID)
{
	TeamID = newTeamID;
}

bool UGameplayAbilitySV::IsNeedTick_Implementation()
{
	return false;
}

void UGameplayAbilitySV::InitAbility_Implementation()
{
}

ACharacterSV* UGameplayAbilitySV::GetCaster() const
{
	return Cast<ACharacterSV>(GetOwningActorFromActorInfo());
}

void UGameplayAbilitySV::HitTestRay(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, float rayLength
	, int targetCount, EAbilityTarget::Type targetType)
{
	ACharacterSV *caster = GetCaster();

	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	FTransform trans;
	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMesh:
	case EEffectPosition::GripMeshAttached:
		if (nullptr == mesh) {
			return;
		}
		trans = mesh->GetSocketTransform(fromSocket);
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActor:
	case EEffectPosition::GripActorAttached:
		if (nullptr == actor) {
			return;
		}
		trans = actor->GetTransform();
	default:
		break;
	}

	FVector rayFrom = trans.GetLocation();
	FVector rayTo = trans.GetRotation().GetForwardVector() * rayLength + rayFrom;

	TArray<FHitResult> hitArr;
	TArray<AActor*> ingoreActor;
	if (UKismetSystemLibrary::LineTraceMulti(fromActor, rayFrom, rayTo, TRACE_FIRE, false, ingoreActor, EDrawDebugTrace::None, hitArr, true))
	{
		for (int j = 0, count = 0; j < hitArr.Num() && count < targetCount; ++j)
		{
			FHitResult hitResult = hitArr[j];
			AActor *target = hitResult.GetActor();
			if (target)
			{
				if (FilterHitTarget(target, targetType)) {
					continue;
				}

				if (HitTarget(hitResult)) {
					++count;
				}				
			}
		}
	}
}

void UGameplayAbilitySV::HitTestSphere(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float radius, float duration
	, int targetCount, EAbilityTarget::Type targetType, EHitTestMode::Mode testMode, float periodInterval, bool drawDebug)
{
	FVector loc = fromActor->GetActorLocation();
	FRotator rot = fromActor->GetActorRotation();
	AHitTestActor *hiter = (AHitTestActor*)GetWorld()->SpawnActor(AHitTestActor::StaticClass(), &loc, &rot);
	if (hiter)
	{
		hiter->OwnerAbility = this;
		hiter->TestMode = testMode;
		hiter->TargetType = targetType;
		hiter->TargetCount = targetCount;
		hiter->Duration = duration;
		hiter->PeriodInterval = periodInterval;
		hiter->DrawDebug = drawDebug;

		if (hiter->InitSphere(fromActor, fromPosition, fromSocket, relativeLocation, radius) 
			&& duration > -0.0001f)	{
			HitTestActors.Add(hiter);			
		}
		else {
			DestroyHitTestActor(hiter);
		}
	}
}

void UGameplayAbilitySV::PlayParticle(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, FRotator relativeRotation)
{
	ACharacterSV *caster = GetCaster();
	if (caster) {
		caster->NetMulticastPlayParticle(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
	}
	/*UAbilitySystemComponent* asc = GetActorInfo().AbilitySystemComponent.Get();
	if (asc)
	{
		UAbilitySystemComponentSV* ascSV = Cast<UAbilitySystemComponentSV>(asc);
		if (ascSV)
		{
			ascSV->NetMulticastPlayParticle(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
		}
	}*/

}

void UGameplayAbilitySV::PlayParticleTrail(int particleID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, FRotator relativeRotation
	, FName toParamName, AActor *toActor, EEffectPosition::Type toPosition, FName toSocket, FVector toOffset)
{
	ACharacterSV *caster = GetCaster();
	if (caster) {
		caster->NetMulticastPlayParticleTrail(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation, toParamName, toOffset);
	}
	/*UAbilitySystemComponent* asc = GetActorInfo().AbilitySystemComponent.Get();
	if (asc)
	{
		UAbilitySystemComponentSV* ascSV = Cast<UAbilitySystemComponentSV>(asc);
		if (ascSV)
		{
			FTransform trans = ascSV->MakeFromTransform(toActor, toPosition, toSocket, toOffset);
			ascSV->NetMulticastPlayParticleTrail(particleID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation, toParamName, toOffset);
		}
	}*/
}

void UGameplayAbilitySV::PlaySound(int soundID, AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, FRotator relativeRotation)
{
	ACharacterSV *caster = GetCaster();
	if (caster) {
		caster->NetMulticastPlaySound(soundID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
	}
	/*UAbilitySystemComponent* asc = GetActorInfo().AbilitySystemComponent.Get();
	if (asc)
	{
		UAbilitySystemComponentSV* ascSV = Cast<UAbilitySystemComponentSV>(asc);
		if (ascSV)
		{
			ascSV->NetMulticastPlaySound(soundID, fromActor, fromPosition, fromSocket, relativeLocation, relativeRotation);
		}
	}*/
}

void UGameplayAbilitySV::SpawnImpactEffects(const FHitResult& hit, TSubclassOf<AHitImpactEffect> impactEffect)
{
	ACharacterSV *caster = GetCaster();
	if (caster) {
		caster->NetMulticastSpawnImpactEffects(hit, impactEffect);
	}
	/*UAbilitySystemComponent* asc = GetActorInfo().AbilitySystemComponent.Get();
	if (asc)
	{
		UAbilitySystemComponentSV* ascSV = Cast<UAbilitySystemComponentSV>(asc);
		if (ascSV)
		{
			ascSV->NetMulticastSpawnImpactEffects(hit, impactEffect);
		}
	}*/
}

bool UGameplayAbilitySV::TryToConsume()
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, actor, mesh);

	if (actor)
	{
		AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
		if (entity)
		{
			return entity->TryToConsume();
		}
	}

	return false;
}

void UGameplayAbilitySV::CalcShakeOffset()
{
	ACharacterSV *caster = GetCaster();
	if (caster) {
		caster->NetMulticastCalcShakeOffset(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh);
	}
	/*UAbilitySystemComponent* asc = GetActorInfo().AbilitySystemComponent.Get();
	if (asc)
	{
		UAbilitySystemComponentSV* ascSV = Cast<UAbilitySystemComponentSV>(asc);
		if (ascSV)
		{
			ascSV->NetMulticastCalcGunRecoilOffset(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh);
		}
	}*/
}

void UGameplayAbilitySV::GunFireTrace(float radius, int trailParticleID, FName trailTargetParam, TArray<FHitResult> &hits, EAbilityTarget::Type targetType, EDrawDebugTrace::Type DrawDebugType, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, actor, mesh);

	if (mesh == nullptr) {
		return;
	}

	AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
	if (entity == nullptr) {
		return;
	}

	FAbilityEntityInfoRow entityParam = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(entity->GetParamID());

	//散布半径
	const int32 randomSeed = FMath::Rand();
	FRandomStream gunRandomStream(randomSeed);
	const float coneHalfAngle = FMath::DegreesToRadians(entity->GetSpread() * 0.5f);

	if (entityParam.TraceSocket.IsNone())
	{
		UKismetSystemLibrary::PrintString(this, TEXT("FireTraceSocket is None!"), true, true, FLinearColor(1, 0, 0));
	}
	FTransform transform = mesh->GetSocketTransform(entityParam.TraceSocket);//entity->GetGunSocketTransform(fireTraceSocket, ERelativeTransformSpace::RTS_World);
	FVector startTrace = transform.GetLocation();
	FVector muzzleRotator = transform.GetRotation().Vector();


	int ballNum = entityParam.PerNum;
	float traceRadius = entityParam.TraceRadius;
	if (traceRadius <= 0) {
		traceRadius = 1.0f;
	}
	for (int i = 0; i < ballNum; ++i)
	{
		//随机方向
		const FVector fireDir = gunRandomStream.VRandCone(muzzleRotator, coneHalfAngle, coneHalfAngle);
		FVector endTrace = startTrace + fireDir * radius;

		TArray<FHitResult> hitArr;
		TArray<AActor*> ingoreActor;
		FVector trailEndPoint = endTrace;
		bool trailEnd = false;
		uint16 curAttactCount = 0;
		
		if (UKismetSystemLibrary::SphereTraceMulti(this, startTrace, endTrace, traceRadius, TRACE_FIRE, false, ingoreActor, DrawDebugType, hitArr, true, TraceColor, TraceHitColor, DrawTime))
		{
			TArray<AActor*> alreadyHitActors;
			for (int j = 0; j < hitArr.Num(); ++j)
			{
				FHitResult hitResult = hitArr[j];
				AActor* hitActor = hitResult.GetActor();
				if (hitActor)
				{
					bool alreadyHit = false;
					for (int k = 0; k < alreadyHitActors.Num(); ++k)
					{
						if (hitActor == alreadyHitActors[k]) {
							alreadyHit = true;
							break;
						}
					}
					if (alreadyHit) {
						continue;
					}
					alreadyHitActors.Add(hitActor);

					//组别
					if (FilterHitTarget(hitActor, targetType)) {
						continue;
					}

					//ability->NotifyOnHit(hitResult);
					hits.Add(hitResult);
					OnHit(hitResult);
					//弹道终点
					if (!trailEnd) {
						trailEndPoint = hitResult.ImpactPoint;
						trailEnd = true;
					}

					++curAttactCount;
					if (curAttactCount >= entityParam.TargetCount) {
						break;
					}
				}
				else {
					//ability->NotifyOnHit(hitResult);
					hits.Add(hitResult);
					OnHit(hitResult);
					break;
				}
			}

		}


		//弹道特效
		PlayParticleTrail(trailParticleID, GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, entityParam.TraceSocket, FVector::ZeroVector, FRotator::ZeroRotator, trailTargetParam,
			GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, entityParam.TraceSocket, trailEndPoint);
	}

	//子弹散布
	entity->SetSpreadTime();
}

void UGameplayAbilitySV::Damage(const FHitResult &hitResult, EAttackType::Type attackType, int damage, int perDamage, bool PlayEffect)
{
	if (!hitResult.bBlockingHit) {
		//return;
	}

	AActor* hitActor = hitResult.GetActor();
	bool playEffect = true;

	if (hitActor)
	{
		if (ACharacterSV* ch = Cast<ACharacterSV>(hitActor))
		{
			do
			{
				int originalDamage = Attribs[EAttribType::AttackPower] + damage + Attribs[EAttribType::AttackPower] * perDamage;
				if (attackType == EAttackType::Projectile) 
				{
					AActor *actor = nullptr;
					UMeshComponent *mesh = nullptr;
					ACharacterSV::GetFromActorAndMesh(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripActor, actor, mesh);

					AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
					EPhysicalSurface surface = UPhysicalMaterial::DetermineSurfaceType(hitResult.PhysMaterial.Get());
					FVector distance = hitResult.ImpactPoint - hitResult.TraceStart;
					originalDamage = ACharacterSV::CalcProjectileDamage(originalDamage, Attribs, ch->GetAttribs(), entity, surface, distance.Size());
				}
				else if (attackType == EAttackType::Melee)
				{
					originalDamage = ACharacterSV::CalcMeleeDamage(originalDamage, Attribs, ch->GetAttribs());
				}
				int health = ch->GetAttrib(EAttribType::Health);

				if (ch->IsMajor() && health <= originalDamage)
				{
					const FGlobalConfigRow *config = &FGlobalConfigRes::Get().GetConfig("DeadBuff");
					if (config && config->IntValue.Num() > 2) 
					{
						ch->AddBuff(config->IntValue[0], config->IntValue[1], config->IntValue[2], Attribs);
						break;
					}
				}

				health -= originalDamage;
				health = FMath::Max(health, 0);
				ch->SetAttrib(EAttribType::Health, health);
				ch->NetMulticastNotifyOnHit(hitResult, originalDamage, ch->GetAttrib(EAttribType::Health), attackType);

				UAISense_Damage::ReportDamageEvent(GetWorld(), ch, GetCaster(), originalDamage, ch->GetActorLocation(), ch->GetActorLocation());

				playEffect = PlayEffect;
			} while (0);
		}
		else 
		{
			FPointDamageEvent PointDmg;
			PointDmg.HitInfo = hitResult;
			PointDmg.ShotDirection = hitResult.TraceEnd - hitResult.TraceStart;
			PointDmg.Damage = damage;

			AActor* ownerActor = GetActorInfo().OwnerActor.Get();
			AController * ac = nullptr;
			
			if (ownerActor)
			{
				APawn* pawn = Cast<APawn>(ownerActor);
				if (pawn) {
					ac = pawn->GetController();
				}
				
			}

			hitActor->TakeDamage(PointDmg.Damage, PointDmg, ac, ownerActor);
		}
	}

	if (playEffect)
	{
		if (AbilityInfo.ImpactEffect)
		{
			SpawnImpactEffects(hitResult, AbilityInfo.ImpactEffect);
		}
	}
}

void UGameplayAbilitySV::AddHP(AActor* targethp, int value)
{
	ACharacterSV* tar = Cast<ACharacterSV>(targethp);
	if (tar)
	{
		tar->AddAttrib(EAttribType::Health, value);
	}
}

void UGameplayAbilitySV::ReportNoiseEvent(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float Loudness, float MaxRange, FName Tag)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(fromActor, fromPosition, actor, mesh);

	FVector loc;
	switch (fromPosition)
	{
	case EEffectPosition::Mesh:
	case EEffectPosition::MeshAttached:
	case EEffectPosition::GripMesh:
	case EEffectPosition::GripMeshAttached:
		if (nullptr == mesh) {
			return;
		}
		loc = mesh->GetSocketLocation(fromSocket);
		break;
	case EEffectPosition::Actor:
	case EEffectPosition::ActorAttached:
	case EEffectPosition::GripActor:
	case EEffectPosition::GripActorAttached:
		if (nullptr == actor) {
			return;
		}
		loc = actor->GetActorLocation();
	default:
		break;
	}

	UAISense_Hearing::ReportNoiseEvent(this, loc, Loudness, fromActor, MaxRange, Tag);
}

void UGameplayAbilitySV::Reload()
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, actor, mesh);

	if (actor)
	{
		AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
		if (entity)
		{
			entity->Reload();
		}
	}
}

void UGameplayAbilitySV::PlayForceFeedback(class UForceFeedbackEffect* ForceFeedbackEffect, bool bLooping, FName Tag)
{
	AActor* actor = GetOwningActorFromActorInfo();
	APawn* pawn = Cast<APawn>(actor);
	if (pawn == nullptr) {
		return;
	}

	AController* controller = pawn->GetController();
	APlayerController* pc = Cast<APlayerController>(controller);
	if (pc == nullptr) {
		return;
	}

	pc->ClientPlayForceFeedback(ForceFeedbackEffect, bLooping, Tag);
}

void UGameplayAbilitySV::BombSphereTrace(AActor *fromActor, EEffectPosition::Type fromPosition, FName fromSocket, FVector relativeLocation, float radius, EAbilityTarget::Type targetType, EDrawDebugTrace::Type DrawDebugType, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	FTransform trans = ACharacterSV::MakeFromTransform(fromActor, fromPosition, fromSocket, relativeLocation);
	FVector loc = trans.GetLocation();

	TArray<FHitResult> hitArr;
	TArray<AActor*> ingoreActor;
	if (UKismetSystemLibrary::SphereTraceMulti(this, loc, loc, radius, TRACE_FIRE, false, ingoreActor, DrawDebugType, hitArr, true, TraceColor, TraceHitColor, DrawTime))
	{
		TArray<AActor*> alreadyHitActors;
		for (int i = 0; i < hitArr.Num(); ++i)
		{
			FHitResult hitResult = hitArr[i];
			AActor* hitActor = hitResult.GetActor();

			if (hitActor)
			{
				bool alreadyHit = false;
				for (int j = 0; j < alreadyHitActors.Num(); ++j)
				{
					if (hitActor == alreadyHitActors[j]) {
						alreadyHit = true;
						break;
					}
				}
				if (alreadyHit) {
					continue;
				}
				alreadyHitActors.Add(hitActor);

				//组别
				if (FilterHitTarget(hitActor, targetType)) {
					continue;
				}

				FHitResult hitTemp;
				TArray<AActor*> ingoreActoritem;
				if (!UKismetSystemLibrary::SphereTraceSingle(this, loc, hitActor->GetActorLocation(), 5, TRACE_BOMB_BLOCK, false, ingoreActoritem, EDrawDebugTrace::ForDuration, hitTemp, true))
				{
					OnHit(hitResult);
				}
			}
			else {
				OnHit(hitResult);
				break;
			}
		}
	}
}

void UGameplayAbilitySV::AddBuff(const FHitResult &hitResult, int BuffID, int Frames, int BuffPeriodicFrames)
{
	AActor* hitActor = hitResult.GetActor();
	if (hitActor)
	{
		if (ACharacterSV* ch = Cast<ACharacterSV>(hitActor)) {
			ch->AddBuff(BuffID, Frames, BuffPeriodicFrames, Attribs);
		}
	}
}

void UGameplayAbilitySV::RemoveBuff(const FHitResult &hitResult, int BuffID)
{
	AActor* hitActor = hitResult.GetActor();
	if (hitActor)
	{
		if (ACharacterSV* ch = Cast<ACharacterSV>(hitActor)) {
			ch->RemoveBuff(BuffID);
		}
	}
}

void UGameplayAbilitySV::NotifyOnHit_Implementation(FHitResult hit)
{

}

bool UGameplayAbilitySV::FilterHitTarget(AActor *target, EAbilityTarget::Type targetType)
{
	ACharacterSV *ch = Cast<ACharacterSV>(target);
	int target_team = ch ? ch->TeamID : NoTeamId;

	bool is_self = target == GetActorInfo().OwnerActor.Get();
	bool is_neutral = target_team == NoTeamId;
	bool is_friend = target_team == TeamID;
	bool is_enemy = target_team != NoTeamId && target_team != TeamID;

	if ((is_self && (targetType & EAbilityTarget::Self) == 0)
		||(is_friend && (targetType & EAbilityTarget::Friend) == 0)
		|| (is_enemy && (targetType & EAbilityTarget::Enemy) == 0)
		|| (is_neutral && (targetType & EAbilityTarget::Neutral) == 0))
	{
		return true;
	}

	return false;
}

void UGameplayAbilitySV::DestroyHitTestActor(class AHitTestActor *hitTestActor)
{
	UWorld *world = GetWorld();
	if (world)
	{
		if (hitTestActor->TimerHandlePeriod.IsValid()) {
			world->GetTimerManager().ClearTimer(hitTestActor->TimerHandlePeriod);
		}

		if (hitTestActor->TimerHandleDuration.IsValid()) {
			world->GetTimerManager().ClearTimer(hitTestActor->TimerHandleDuration);
		}
	}

	HitTestActors.Remove(hitTestActor);

	hitTestActor->Destroy();
}

bool UGameplayAbilitySV::HitTarget(const FHitResult &hitResult)
{
	AActor *target = hitResult.GetActor();
	//if (AbilityInfo.AbilityType & EAttribType::HitOnce)
	{
		if (HitTargets.Find(target)) {
			return false;
		}
	}

	HitTargets.Add(target);

	//if (hitResult.bBlockingHit)

	OnHit(hitResult);

	return true;
}

void UGameplayAbilitySV::SpawnProjectile(TSubclassOf<AAbilityProjectile> projectile, EAbilityProjectile::Type projectileType, int time, ACharacterSV* target)
{
	AActor *actor = nullptr;
	UMeshComponent *mesh = nullptr;
	ACharacterSV::GetFromActorAndMesh(GetActorInfo().OwnerActor.Get(), EEffectPosition::GripMesh, actor, mesh);
	if (!actor || !mesh)
	{
		return;
	}

	AAbilityEntity* entity = Cast<AAbilityEntity>(actor);
	if (!entity)
	{
		return;
	}

	FActorSpawnParameters spawnInfo;
	FVector loc = entity->GetTraceWorldLocation();
	FRotator rot = entity->GetTraceWorldRotation();
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAbilityProjectile* pro = GetWorld()->SpawnActor<AAbilityProjectile>(projectile, loc, rot, spawnInfo);
	if (pro)
	{
		pro->Init(loc, rot);

		switch (projectileType)
		{
		case EAbilityProjectile::Spline:
		{
			AActor* owning = GetOwningActorFromActorInfo();
			ACharacterSV* ch = Cast<ACharacterSV>(owning);
			if (ch == nullptr) {
			return;
			}
			//ch->ActiveSpline(true, GetAbilityValue());
			pro->SetProjectileVelocity(ch->GetSplineLaunchVelocity(), time/100.0f);
			pro->ActivateProjectile(EAbilityProjectile::Spline);
			//ch->ActiveSpline(false, 0);
		}
			break;
		case EAbilityProjectile::Homing:
			pro->ActivationTraceMode(target);
			pro->ActivateProjectile(EAbilityProjectile::Homing);
			break;
		default:
			break;
		}
	}
	return ;
}