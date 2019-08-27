// Fill out your copyright notice in the Description page of Project Settings.

#include "SvGame.h"
#include "AbilityEntity.h"
#include "PlayerControllerSV.h"
#include "Character/CharacterSV.h"

#include "Res/ShakeRes.h"
#include "Res/SpreadRes.h"
#include "Res/AbilityEntityInfoRes.h"

#include "Net/UnrealNetwork.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

AAbilityEntity::AAbilityEntity()
{
	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SplineComp->SetupAttachment(RootComponent);
}

void AAbilityEntity::BeginPlay()
{
	Super::BeginPlay();

	FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);

	if (EntityType == EEntityType::Gun)
	{
		CurNumInGroup = param.MaxInGroup;
		CurNumInPak = param.MaxInPak;

		GoldShakelOffset = FTransform::Identity;
		CurrentShakeOffset = GoldShakelOffset;
	}
	

	// AAbilityEntity transform is local updated in AMotionControllerProp, so Host must tell client the transform;
	if (GetGameInstance())
	{
		APlayerController *controller = GetGameInstance()->GetFirstLocalPlayerController();
		if (controller && controller->Role == ROLE_AutonomousProxy) {
			((APlayerControllerSV*)controller)->ServerReplicateTrans(this);
		}
	}
}

void AAbilityEntity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAbilityEntity, CurNumInGroup);
	DOREPLIFETIME(AAbilityEntity, CurNumInPak);
}

void AAbilityEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EntityType == EEntityType::Gun)
	{
		FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
		FSpreadRow spread = SpreadRes::Get().GetSpread(param.SpreadID);
		FShakeRow shake = ShakeRes::Get().GetShake(param.ShakeID);

		// 散布恢复
		if (spread.SpreadCurve)
		{
			float fMinTime, fMaxTime;
			spread.SpreadCurve->GetTimeRange(fMinTime, fMaxTime);
			SpreadTime = FMath::Clamp(SpreadTime, fMinTime, fMaxTime);

			if (SpreadTime > 0.0f)
			{
				if (LaggyRecoverTime > 0.0f) 
				{
					LaggyRecoverTime -= DeltaTime;
					if (LaggyRecoverTime <= 0.0f) {
						LaggyRecoverTime = 0.0f;
					}
				}
				else
				{
					SpreadTime -= DeltaTime * spread.SpreadRecover;
					if (SpreadTime <= 0.0f) {
						SpreadTime = 0.0f;
					}
				}
			}

			CurSpread = spread.SpreadCurve->GetFloatValue(SpreadTime);
		}

		//后坐力变化
		if (ApplyShake)
		{
			if (shake.ShakeCurve)
			{
				float fMinTime, fMaxTime;
				shake.ShakeCurve->GetTimeRange(fMinTime, fMaxTime);
				if (ShakeTime >= fMaxTime)
				{
					ShakeTime = 0.0f;
					ApplyShake = false;
				}
				else
				{
					float fShakeValue = shake.ShakeCurve->GetFloatValue(ShakeTime);
					CurrentShakeOffset.Blend(CurrentShakeOffset, GoldShakelOffset, fShakeValue);
					//CurrentShakeOffset = UKismetMathLibrary::TLerp(CurrentShakeOffset, GoldShakelOffset, fRecoilValue);
					ShakeTime += DeltaTime;
				}

			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("No ShakeCurve!"));
				CurrentShakeOffset = GoldShakelOffset;
				ApplyShake = false;
			}
		}

		//后坐力恢复
		if (!ShakeRecoveryFinished)
		{
			float fInterpSpeed = FMath::Clamp<float>(0.5f / UGameplayStatics::GetGlobalTimeDilation(this), 1.0f, 1000000.0f);
			fInterpSpeed *= shake.RecoveryInterpSpeed;

			CurrentShakeOffset.Blend(CurrentShakeOffset, FTransform::Identity, FMath::Clamp<float>(DeltaTime * fInterpSpeed, 0.0f, 1.0f));
			//CurrentShakeOffset = UKismetMathLibrary::TInterpTo(CurrentShakeOffset, FTransform::Identity, DeltaTime, fInterpSpeed);
			ShakeRecoveryFinished = ApplyShake && FTransform::AreRotationsEqual(CurrentShakeOffset, FTransform::Identity, 0.0001)
				&& FTransform::AreTranslationsEqual(CurrentShakeOffset, FTransform::Identity, 0.01);//UKismetMathLibrary::NearlyEqual_TransformTransform(CurrentShakeOffset, FTransform::Identity, 0.01f, 0.0001f, 0.0001f);
		}
	}
}

void AAbilityEntity::PickedUpStart(APawn* Pawn)
{
	Super::PickedUpStart(Pawn);

	ACharacterSV* character = Cast<ACharacterSV>(Pawn);
	if (character != nullptr && character->Role == ROLE_Authority)
	{
		for (auto &elem : Abilitys) {
			character->AddAbility(elem.Key, elem.Value);
		}

		/*FAbilityEntityInfoRow info = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
		if (info.IsSpline) {
			character->ActiveSpline(true, info.SplineVelocity, info.ShowSplineMesh);
		}*/
	}
}

void AAbilityEntity::PickedUpEnd_Implementation()
{
	Super::PickedUpEnd_Implementation();

	ACharacterSV* character = Cast<ACharacterSV>(Instigator);
	if (character && character->Role == ROLE_Authority)
	{
		FAbilityEntityInfoRow info = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
		if (info.IsSpline) 
		{
			character->ActiveSpline(true, info.SplineVelocity, info.ShowSplineMesh);
		}
	}
}

void AAbilityEntity::ThrownAway(bool PutInPackage)
{
	ACharacterSV* character = Cast<ACharacterSV>(Instigator);
	if (character != nullptr)
	{
		if (character->Role == ROLE_Authority)
		{
			for (auto &elem : Abilitys) 
			{
				character->ClearAbilityByID(elem.Value);
			}

			FAbilityEntityInfoRow info = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
			if (info.IsSpline)
			{
				character->ActiveSpline(false);
			}
		}
		
	}

	Super::ThrownAway();
}

void AAbilityEntity::SetCurData(int data1, int data2)
{
	if (data1 >= 0 && data2 >= 0)
	{
		CurNumInGroup = data1;
		CurNumInPak = data2;
	}
}

int AAbilityEntity::GetParamID()
{
	return ParamID;
}

FTransform AAbilityEntity::GetShakeOffset()
{
	return CurrentShakeOffset;
}

void AAbilityEntity::SetShakeOffset(FTransform Offset)
{
	CurrentShakeOffset = Offset;
}

void AAbilityEntity::SetGoldShakeOffset(FTransform gold)
{
	GoldShakelOffset = gold;

	PrimaryActorTick.bCanEverTick = true;
	ApplyShake = true;
	ShakeRecoveryFinished = false;
}

float AAbilityEntity::GetSpread()
{
	return CurSpread;
}

void AAbilityEntity::SetSpread(float spread)
{
	CurSpread = spread;
}

void AAbilityEntity::SetSpreadTime()
{
	FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
	FSpreadRow spread = SpreadRes::Get().GetSpread(param.SpreadID);
	if (spread.SpreadCurve)
	{
		SpreadTime += spread.SpreadIncrement;
		LaggyRecoverTime = spread.LaggyRecoverTime;
	}
}

int AAbilityEntity::GetCurNumInGroup()
{
	return CurNumInGroup;
}

int AAbilityEntity::GetCurNumInpak()
{
	return CurNumInPak;
}

FVector AAbilityEntity::GetTraceWorldLocation_Implementation()
{
	if (PropBody)
	{
		FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
		return PropBody->GetSocketLocation(param.TraceSocket);
	}
	return GetActorLocation();
}

FRotator AAbilityEntity::GetTraceWorldRotation_Implementation()
{
	if (PropBody)
	{
		FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);
		return PropBody->GetSocketRotation(param.TraceSocket);
	}
	return GetActorRotation();
}

bool AAbilityEntity::TryToConsume()
{
	if (EntityType == EEntityType::Gun)
	{
		//使用子弹
		--CurNumInGroup;
		ChangeData.Broadcast(this);

		if (CurNumInGroup < 0) {
			CurNumInGroup = 0;
			return false;
		}

	}
	

	return true;
}

void AAbilityEntity::Reload()
{
	FAbilityEntityInfoRow param = AbilityEntityInfoRes::Get().GetAbilityEntityInfo(ParamID);

	//若背包里没有子弹,或者弹匣中子弹大于等于弹夹最大子弹，则不执行
	if (CurNumInPak == 0 || CurNumInGroup >= param.MaxInGroup) {
		return;
	}

	if (CurNumInPak + CurNumInGroup <= param.MaxInGroup)
	{
		CurNumInGroup += CurNumInPak;
		CurNumInPak = 0;
	}
	else
	{
		int add = param.MaxInGroup - CurNumInGroup;
		CurNumInGroup += add;
		CurNumInPak -= add;
	}

	ChangeData.Broadcast(this);
}

bool AAbilityEntity::NeedReload()
{
	return CurNumInGroup == 0 && CurNumInPak > 0;
}